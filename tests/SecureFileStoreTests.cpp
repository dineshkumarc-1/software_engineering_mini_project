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
    "SecureFileStore safely handles fuzzed and truncated JSON",
    "[persistence][store][fuzz]") {

    TemporaryDirectory directory;

    const auto file = directory.path() / "fuzz-input.json";
    const snake::SecureFileStore store(file);

    const std::string validJson =
        R"({"version":1,"entries":[{"player_name":"Alice","score":10,"game_mode":"Classic","difficulty":"Medium","date_achieved":"2026-10-28T12:00:00Z"}]})";

    // Write each input and parse it. Report file setup errors
    // through the normal result so the test fails visibly.
    const auto parsePayload = [&](const std::string& payload)
        -> snake::StoreReadResult {

        std::ofstream output(
            file,
            std::ios::binary | std::ios::trunc);

        if (!output) {
            return {
                snake::StoreReadStatus::IoError,
                {},
                "Could not create fuzz-test input."
            };
        }

        output.write(
            payload.data(),
            static_cast<std::streamsize>(payload.size()));
        output.close();

        if (output.fail()) {
            return {
                snake::StoreReadStatus::IoError,
                {},
                "Could not write fuzz-test input."
            };
        }

        return store.readJson();
    };

    SECTION("Every strict prefix of valid JSON is rejected") {
        for (std::size_t length = 0;
             length < validJson.size();
             ++length) {

            const auto result =
                parsePayload(validJson.substr(0, length));

            if (length == 0U) {
                CHECK(
                    result.status ==
                    snake::StoreReadStatus::EmptyFile);
            } else {
                CHECK(
                    result.status ==
                    snake::StoreReadStatus::InvalidJson);
            }
        }
    }

    SECTION("A control-character mutation at every byte is rejected") {
        for (std::size_t index = 0;
             index < validJson.size();
             ++index) {

            std::string mutated = validJson;
            mutated[index] = '\x01';

            const auto result = parsePayload(mutated);

            CHECK(
                result.status ==
                snake::StoreReadStatus::InvalidJson);
        }
    }

    SECTION("Malformed payload corpus is handled safely") {
        const std::string malformedPayloads[] = {
            "{",
            "{\"entries\":",
            "{\"entries\":[}",
            "{\"value\":tru}",
            "[1,2,",
            "{\"name\":\"unterminated"
        };

        for (const auto& payload : malformedPayloads) {
            const auto result = parsePayload(payload);

            CHECK(
                result.status ==
                snake::StoreReadStatus::InvalidJson);
        }
    }
}