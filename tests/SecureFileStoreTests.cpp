#include "snakextreme/SecureFileStore.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        static std::atomic<unsigned int> counter{0};

        path_ = std::filesystem::temp_directory_path() /
            ("snakextreme_store_test_" +
             std::to_string(counter.fetch_add(1)));

        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    [[nodiscard]]
    const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

} // namespace

TEST_CASE(
    "SecureFileStore reports a missing file",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const snake::SecureFileStore store(
        directory.path() / "missing.json");

    const auto result = store.readJson();

    CHECK(result.status == snake::StoreReadStatus::NotFound);
}

TEST_CASE(
    "SecureFileStore rejects directories",
    "[persistence][store][security]") {

    TemporaryDirectory directory;

    const auto folder = directory.path() / "folder";
    REQUIRE(std::filesystem::create_directory(folder));

    const snake::SecureFileStore store(folder);

    CHECK(
        store.readJson().status ==
        snake::StoreReadStatus::UnsafeFile);
}

TEST_CASE(
    "SecureFileStore rejects symbolic links",
    "[persistence][store][security]") {

    TemporaryDirectory directory;

    const auto target = directory.path() / "target.json";
    const auto link = directory.path() / "link.json";

    {
        std::ofstream output(target, std::ios::binary);
        REQUIRE(output.good());
        output << R"({"value":42})";
    }

    std::error_code ec;
    std::filesystem::create_symlink(target, link, ec);

    // Some Windows environments do not permit creating
    // symbolic links without additional privileges.
    if (ec) {
        WARN("Could not create symbolic link in this environment: "
             << ec.message());
        return;
    }

    const snake::SecureFileStore store(link);

    CHECK(
        store.readJson().status ==
        snake::StoreReadStatus::UnsafeFile);
}

TEST_CASE(
    "SecureFileStore refuses writes through symbolic links",
    "[persistence][store][security]") {

    TemporaryDirectory directory;

    const auto target = directory.path() / "target.json";
    const auto link = directory.path() / "link.json";

    const std::string originalContents = R"({"value":42})";

    {
        std::ofstream output(target, std::ios::binary);
        REQUIRE(output.good());
        output << originalContents;
    }

    std::error_code ec;
    std::filesystem::create_symlink(target, link, ec);

    if (ec) {
        WARN("Could not create symbolic link in this environment: "
             << ec.message());
        return;
    }

    const snake::SecureFileStore store(link);

    const nlohmann::json replacement = {
        {"value", 99}
    };

    CHECK_FALSE(store.writeJson(replacement));

    // The target must remain unchanged.
    std::ifstream input(target, std::ios::binary);
    REQUIRE(input.good());

    const std::string contentsAfter(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    CHECK(contentsAfter == originalContents);
}



TEST_CASE(
    "SecureFileStore writes and reads JSON",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const snake::SecureFileStore store(
        directory.path() / "data.json");

    const nlohmann::json original{
        {"value", 42},
        {"enabled", true}
    };

    REQUIRE(store.writeJson(original));

    const auto result = store.readJson();

    REQUIRE(result.status == snake::StoreReadStatus::Ok);
    CHECK(result.value == original);
}

TEST_CASE(
    "SecureFileStore rejects malformed JSON",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "broken.json";

    {
        std::ofstream output(file, std::ios::binary);
        output << "{ this is not JSON";
    }

    const snake::SecureFileStore store(file);

    CHECK(
        store.readJson().status ==
        snake::StoreReadStatus::InvalidJson);
}

TEST_CASE(
    "SecureFileStore rejects duplicate JSON object keys",
    "[persistence][store][security]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "duplicate.json";

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output << R"({"score":10,"score":99})";
    }

    const snake::SecureFileStore store(file);
    const auto result = store.readJson();

    CHECK(result.status == snake::StoreReadStatus::UnsafeJson);
}

TEST_CASE(
    "SecureFileStore rejects JSON nested beyond the depth limit",
    "[persistence][store][security]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "deep.json";

    // The configured limit is 64 nested structures.
    std::string deeplyNested(65U, '[');
    deeplyNested += "0";
    deeplyNested.append(65U, ']');

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output << deeplyNested;
    }

    const snake::SecureFileStore store(file);
    const auto result = store.readJson();

    CHECK(result.status == snake::StoreReadStatus::UnsafeJson);
}
TEST_CASE(
    "SecureFileStore supports a stricter caller depth limit",
    "[persistence][store][security]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "limited-depth.json";

    std::string nested(9U, '[');
    nested += "0";
    nested.append(9U, ']');

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output << nested;
    }

    const snake::SecureFileStore store(file);

    CHECK(store.readJson().status == snake::StoreReadStatus::Ok);

    CHECK(
        store.readJson(8U).status ==
        snake::StoreReadStatus::UnsafeJson);
}

TEST_CASE(
    "SecureFileStore enforces the 64 KiB read cap",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "large.json";

    {
        std::ofstream output(file, std::ios::binary);

        output << std::string(
            snake::SecureFileStore::MaxFileSizeBytes + 1U,
            'x');
    }

    const snake::SecureFileStore store(file);

    CHECK(
        store.readJson().status ==
        snake::StoreReadStatus::TooLarge);
}

TEST_CASE(
    "SecureFileStore rejects JSON writes above 64 KiB",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "large.json";

    const snake::SecureFileStore store(file);

    const nlohmann::json large{
        {"data", std::string(70U * 1024U, 'x')}
    };

    CHECK_FALSE(store.writeJson(large));
    CHECK_FALSE(std::filesystem::exists(file));
}
TEST_CASE(
    "SecureFileStore accepts a file exactly 64 KiB",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "exact.json";
    const auto limit =
        snake::SecureFileStore::MaxFileSizeBytes;

    const std::string prefix = "{\"padding\":\"";
    const std::string suffix = "\"}";

    const std::string bytes =
        prefix +
        std::string(limit - prefix.size() - suffix.size(), ' ') +
        suffix;

    REQUIRE(bytes.size() == limit);

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());

        output.write(
            bytes.data(),
            static_cast<std::streamsize>(bytes.size()));

        REQUIRE(output.good());
    }

    const snake::SecureFileStore store(file);
    const auto result = store.readJson();

    CHECK(result.status == snake::StoreReadStatus::Ok);
}
TEST_CASE(
    "SecureFileStore accepts JSON serialized to exactly 64 KiB",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "exact-write.json";
    const snake::SecureFileStore store(file);
    const auto limit =
        snake::SecureFileStore::MaxFileSizeBytes;

    const nlohmann::json empty = {{"data", ""}};
    const auto baseSize = empty.dump(2).size();

    REQUIRE(baseSize < limit);

    const nlohmann::json exact = {
        {"data", std::string(limit - baseSize, 'x')}
    };

    REQUIRE(exact.dump(2).size() == limit);
    REQUIRE(store.writeJson(exact));

    CHECK(std::filesystem::file_size(file) == limit);
}
TEST_CASE(
    "Oversized write preserves the previous valid file",
    "[persistence][store]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "preserve.json";
    const snake::SecureFileStore store(file);

    const nlohmann::json original = {
        {"value", 42}
    };

    REQUIRE(store.writeJson(original));

    const nlohmann::json oversized = {
        {"data", std::string(70U * 1024U, 'x')}
    };

    CHECK_FALSE(store.writeJson(oversized));

    const auto result = store.readJson();

    REQUIRE(result.status == snake::StoreReadStatus::Ok);
    CHECK(result.value == original);
}

TEST_CASE(
    "SecureFileStore removes only stale matching temp files and leaves unrelated files and symlinks",
    "[persistence][store][security]") {

    TemporaryDirectory directory;
    const auto destination = directory.path() / "data.json";
    const snake::SecureFileStore store(destination);
    REQUIRE(store.writeJson(nlohmann::json{{"value", 42}}));

    const auto stale = directory.path() / "data.json.tmp.12345";
    const auto unrelated = directory.path() / "other.json.tmp.12345";

    {
        std::ofstream output(stale, std::ios::binary);
        REQUIRE(output.good());
        output << R"({"partial":)";
    }
    {
        std::ofstream output(unrelated, std::ios::binary);
        REQUIRE(output.good());
        output << "unrelated";
    }

    const auto oldTime =
        std::filesystem::file_time_type::clock::now() -
        std::chrono::hours(48);
    std::error_code timeError;
    std::filesystem::last_write_time(stale, oldTime, timeError);
    REQUIRE_FALSE(timeError);
    std::filesystem::last_write_time(unrelated, oldTime, timeError);
    REQUIRE_FALSE(timeError);

    const auto target = directory.path() / "link-target.json";
    const auto link = directory.path() / "data.json.tmp.54321";
    {
        std::ofstream output(target, std::ios::binary);
        REQUIRE(output.good());
        output << R"({"value":7})";
    }

    std::error_code symlinkError;
    std::filesystem::create_symlink(target, link, symlinkError);

    const auto result = store.readJson();
    REQUIRE(result.status == snake::StoreReadStatus::Ok);
    CHECK_FALSE(std::filesystem::exists(stale));
    CHECK(std::filesystem::exists(unrelated));

    if (!symlinkError) {
        CHECK(std::filesystem::is_symlink(
            std::filesystem::symlink_status(link)));
    } else {
        WARN("Symbolic links unavailable in this environment: "
             << symlinkError.message());
    }
}

TEST_CASE(
    "SecureFileStore preserves a stale temp file while another writer holds its lock",
    "[persistence][store][security]") {

    TemporaryDirectory directory;
    const auto destination = directory.path() / "data.json";
    const snake::SecureFileStore store(destination);
    REQUIRE(store.writeJson(nlohmann::json{{"value", 42}}));

    const auto stale = directory.path() / "data.json.tmp.67890";
    const auto lockPath = directory.path() / "data.json.tmp.67890.lock";

    {
        std::ofstream output(stale, std::ios::binary);
        REQUIRE(output.good());
        output << "partial write";
    }

    std::error_code timeError;
    std::filesystem::last_write_time(
        stale,
        std::filesystem::file_time_type::clock::now() -
            std::chrono::hours(48),
        timeError);
    REQUIRE_FALSE(timeError);

#ifdef _WIN32
    HANDLE activeLock = ::CreateFileW(
        lockPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    REQUIRE(activeLock != INVALID_HANDLE_VALUE);
#else
    const int activeLock = ::open(
        lockPath.c_str(),
        O_RDWR | O_CREAT | O_EXCL,
        S_IRUSR | S_IWUSR);
    REQUIRE(activeLock >= 0);
    REQUIRE(::flock(activeLock, LOCK_EX | LOCK_NB) == 0);
#endif

    const auto firstRead = store.readJson();
    REQUIRE(firstRead.status == snake::StoreReadStatus::Ok);
    CHECK(std::filesystem::exists(stale));

#ifdef _WIN32
    REQUIRE(::CloseHandle(activeLock));
#else
    REQUIRE(::flock(activeLock, LOCK_UN) == 0);
    REQUIRE(::close(activeLock) == 0);
#endif

    const auto secondRead = store.readJson();
    REQUIRE(secondRead.status == snake::StoreReadStatus::Ok);
    CHECK_FALSE(std::filesystem::exists(stale));
    CHECK_FALSE(std::filesystem::exists(lockPath));
}

TEST_CASE(
    "SecureFileStore failed replacement preserves the destination and cleans its temp files",
    "[persistence][store][security]") {

    TemporaryDirectory directory;
    const auto destination = directory.path() / "locked.json";
    const snake::SecureFileStore store(destination);
    const nlohmann::json original = {{"value", 42}};
    REQUIRE(store.writeJson(original));

#ifdef _WIN32
    HANDLE targetLock = ::CreateFileW(
        destination.c_str(),
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    REQUIRE(targetLock != INVALID_HANDLE_VALUE);

    CHECK_FALSE(store.writeJson(nlohmann::json{{"value", 99}}));
    REQUIRE(::CloseHandle(targetLock));
#else
    // On POSIX, the portable fault injection is a non-writable parent.
    // The existing destination remains readable; no temp is staged.
    if (::geteuid() == 0) {
        SKIP("Root ignores directory permissions; CI should run this as an unprivileged user");
    }
    std::filesystem::permissions(
        directory.path(),
        std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::replace);
    CHECK_FALSE(store.writeJson(nlohmann::json{{"value", 99}}));
    std::filesystem::permissions(
        directory.path(),
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::add);
#endif

    const auto result = store.readJson();
    REQUIRE(result.status == snake::StoreReadStatus::Ok);
    CHECK(result.value == original);

    for (const auto& item : std::filesystem::directory_iterator(directory.path())) {
        const auto name = item.path().filename().string();
        CHECK(name.find("locked.json.tmp.") != 0U);
    }
}
