// Next up, add --install flag to copy exe to paths.directory and autostart?
// After that, put the bedtimes entries in different directory in task scheduler
// And add convenience function for enabling/disabling all of them?

#include "godnatt.h"

#include "common.cpp"
#include "paths.cpp"
#include "bedtimes.cpp"
#include "taskscheduler.cpp"
#include "filewatching.cpp"

bool directory_exists(const char *path)
{
    DWORD attrib = GetFileAttributesA(path);

    return (attrib != INVALID_FILE_ATTRIBUTES) &&
           (attrib & FILE_ATTRIBUTE_DIRECTORY);
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "--lock") == 0)
        {
            if (!LockWorkStation())
            {
                ShowError("[ERROR::Lock Workstation] Failed with error code: %x\n", GetLastError());
                return 2;
            }
        }
        else if (strcmp(argv[1], "--notify") == 0)
        {
            ShowError("Computer will get locked in 5 minutes.\nSave your work so you don't lose any!");
        }
    }
    else
    {
        // TODO: Check if bedtimes file exists separately in case it gets renamed or removed
        if (!directory_exists(paths.directory))
        {
            const char *defaultBedtimes =
                "# Example bedtime file\r\n"
                "# Week starts on monday since I'm swedish\r\n"
                "# Just a single entry per day.\r\n"
                "# No more, no less.\r\n"
                "21:45\r\n"
                "21:45\r\n"
                "21:45\r\n"
                "21:45\r\n"
                "23:45\r\n"
                "23:45\r\n"
                "21:45";

            // if main directory doesn't exist, create it and add the default bedtimes file
            if (CreateDirectoryA(paths.directory, nullptr) == 0)
            {
                ShowError("[ERROR::Create directory] CreateDirectoryA failed with error code: %x\n", GetLastError());
                return 1;
            }

            FILE *file = nullptr;
            errno_t error = fopen_s(&file, paths.bedtimes, "wb");
            if (error != 0)
            {
                return 1;
            }

            size_t numBytesToWrite = strlen(defaultBedtimes);
            if (fwrite(defaultBedtimes, 1, numBytesToWrite, file) != numBytesToWrite)
            {
                ShowError("[ERROR::Create default bedtimes] fwrite failed\n");
                fclose(file);
                return 1;
            }

            fclose(file);
        }

        // Default to watch bedtimes file for changes
        return WatchFile(paths.bedtimes);
    }
}