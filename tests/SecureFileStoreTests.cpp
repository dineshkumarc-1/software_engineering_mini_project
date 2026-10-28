#include "snakextreme/SecureFileStore.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
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