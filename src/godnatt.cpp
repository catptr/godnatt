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
        else if (strcmp(argv[1], "--clear") == 0)
        {
            TaskScheduler ts;
            if (!ts.did_succeed())
            {
                return 1;
            }

            ts.clear();

            if (!ts.did_succeed())
            {
                return 1;
            }
        }
        else if (strcmp(argv[1], "--edit") == 0)
        {
            STARTUPINFO si = {};
            PROCESS_INFORMATION pi = {};
            si.cb = sizeof(si);

            // cast away constness so CreateProcess isn't sad
            char *cmd = (char *)concat_path("notepad.exe ", paths.bedtimes);

            if (!CreateProcessA(nullptr,
                cmd,
                nullptr,
                nullptr,
                false,
                0,
                nullptr,
                nullptr,
                &si,
                &pi)
            )
            {
                free(cmd);
                ShowError("[ERROR::Edit] CreateProcess failed (%d).\n", GetLastError());
                return 1;
            }

            free(cmd);

            // Close process and thread handles.
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else if (strcmp(argv[1], "--install") == 0)
        {
            // Kill godnatt.exe then copy from debug to release dirs
            // Maybe make sure there's a shortcut in autostart?
            ShowError("Not implemented.");
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
        return WatchFile(paths.directory, paths.bedtimes);
    }

    return 0;
}