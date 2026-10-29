// Fixture for the banned-API scan's self-test (cmake/CheckBannedApis.cmake).
// Not compiled. Every call below must be reported: 12 hits.
#include <cstdio>
#include <cstdlib>
#include <cstring>

void runCommands(const char* fromFile) {
    std::system(fromFile);
    FILE* pipe = popen(fromFile, "r");
    execvp(fromFile, nullptr);
    void* library = dlopen(fromFile, 0);
    LoadLibraryA(fromFile);
}

void unsafeStrings(char* out, const char* fromFile) {
    strcpy(out, fromFile);
    strcat(out, fromFile);
    sprintf(out, "%s", fromFile);
    gets(out);
}

void formats(const char* fromFile) {
    printf(fromFile);
    std::fprintf(stderr, fromFile);
    snprintf(nullptr, 0, fromFile);
}
