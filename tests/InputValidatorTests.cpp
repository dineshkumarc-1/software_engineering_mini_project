#include "snakextreme/InputValidator.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using snake::isValidPlayerName;
using snake::sanitizePlayerName;

TEST_CASE("Accepts player names containing allowed characters",
          "[validation][player-name]") {
    CHECK(isValidPlayerName("Bharath"));
    CHECK(isValidPlayerName("Player123"));
    CHECK(isValidPlayerName("Player_1"));
    CHECK(isValidPlayerName("Player-1"));
    CHECK(isValidPlayerName("Cool Player-1"));
}

TEST_CASE("Rejects empty names and disallowed characters",
          "[validation][player-name]") {
    CHECK_FALSE(isValidPlayerName(""));
    CHECK_FALSE(isValidPlayerName("Player@123"));
    CHECK_FALSE(isValidPlayerName("Player#1"));
    CHECK_FALSE(isValidPlayerName("Player.1"));
    CHECK_FALSE(isValidPlayerName("Player\n1"));
}

TEST_CASE("Enforces the 15-character limit",
          "[validation][player-name]") {
    const std::string fifteenCharacters(15, 'A');
    const std::string sixteenCharacters(16, 'A');

    CHECK(isValidPlayerName(fifteenCharacters));
    CHECK_FALSE(isValidPlayerName(sixteenCharacters));
}

TEST_CASE("Sanitization removes disallowed characters",
          "[validation][player-name]") {
    CHECK(sanitizePlayerName("Player@123") == "Player123");
    CHECK(sanitizePlayerName("Cool Player-1") == "Cool Player-1");
    CHECK(sanitizePlayerName("@#$").empty());
}

TEST_CASE("Sanitization never returns more than 15 characters",
          "[validation][player-name]") {
    const std::string input = "1234567890123456";

    CHECK(sanitizePlayerName(input) == "123456789012345");
    CHECK(sanitizePlayerName(input).size() == 15);
}

TEST_CASE("Rejects empty and whitespace-only player names",
          "[validation][player-name]") {
    CHECK_FALSE(isValidPlayerName(""));
    CHECK_FALSE(isValidPlayerName(" "));
    CHECK_FALSE(isValidPlayerName("     "));
}

TEST_CASE("Trims leading and trailing spaces from padded names",
          "[validation][player-name]") {
    CHECK(isValidPlayerName("  Bharath  "));
    CHECK(sanitizePlayerName("  Bharath  ") == "Bharath");

    CHECK(isValidPlayerName("Cool Player"));
    CHECK(sanitizePlayerName("  Cool Player  ") == "Cool Player");

    CHECK(sanitizePlayerName("     ").empty());
}

TEST_CASE("Enforces the 15-character limit after trimming",
          "[validation][player-name]") {
    const std::string fifteenCharacters(15, 'A');
    const std::string sixteenCharacters(16, 'A');

    CHECK(isValidPlayerName("  " + fifteenCharacters + "  "));
    CHECK(sanitizePlayerName("  " + fifteenCharacters + "  ")
          == fifteenCharacters);

    CHECK_FALSE(isValidPlayerName("  " + sixteenCharacters + "  "));
    CHECK(sanitizePlayerName("  " + sixteenCharacters + "  ")
          == fifteenCharacters);
}

TEST_CASE("Character-level validation permits spaces but rejects symbols",
          "[validation][player-name]") {
    CHECK(snake::isAllowedPlayerNameCharacter(' '));
    CHECK(snake::isAllowedPlayerNameCharacter('A'));
    CHECK(snake::isAllowedPlayerNameCharacter('_'));
    CHECK_FALSE(snake::isAllowedPlayerNameCharacter('@'));
    CHECK_FALSE(snake::isAllowedPlayerNameCharacter('#'));
}
