#include "paths.h"

const char *concat_path(const char *directory, const char *file)
{
    size_t length = strlen(directory) + strlen(file) + 1;
    char *result = (char *)malloc(sizeof(char) * length);

    strcpy_s(result, length, directory);
    strcat_s(result, length, file);

    return result;
}

Paths::Paths()
{
#ifdef DEBUG
    this->directory = "G:\\godnatt\\";
#else
    const char *godnatt = "\\godnatt\\";
    const size_t godnattLength = strlen(godnatt);

    // This is kinda bad since we use ansi strings when the appdata path will contain a user name which
    // can contain unicode characters.
    DWORD varLength = GetEnvironmentVariableA("APPDATA", nullptr, 0);
    size_t length = varLength + godnattLength;
    // length includes null terminator but we want to add a backslash so we still add 1
    char *directory = (char *)malloc(sizeof(char) * length);
    GetEnvironmentVariableA("APPDATA", directory, varLength);
    strcat_s(directory, length, "\\godnatt\\");

    this->directory = directory;
#endif

    this->bedtimes = concat_path(this->directory, "bedtimes.txt");
    this->lastUpdate = concat_path(this->directory, "last_update");
    this->executable = concat_path(this->directory, "godnatt.exe");
}

Paths paths;