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
    this->directory = "G:\\godnatt\\";
    this->bedtimes = concat_path(this->directory, "bedtimes.txt");
    this->lastUpdate = concat_path(this->directory, "last_update");
}

Paths paths;