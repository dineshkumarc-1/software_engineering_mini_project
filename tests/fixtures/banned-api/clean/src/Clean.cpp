// Fixture for the banned-API scan's self-test (cmake/CheckBannedApis.cmake).
// Not compiled. Nothing here may be reported.
#include <filesystem>
#include <string>

// Mentions in comments are fine: system(), popen(), printf("%s").
/* A block comment too:
   sprintf(buffer, format);
*/
void lookalikes(std::FILE* file, char* line) {
    std::filesystem::path where("data");
    fgets(line, 16, file);            // fgets is bounded
    const std::string name = "system";
    ecosystem(name);
    mySystem(name);
    prints(name);
    std::string text = "call printf( here";
    shellExecuteLater();
    std::snprintf(line, 16, "%d", 3); // banned-api-ok: reviewed fixture line
}
