#include "godnatt.h"

#include "common.cpp"
#include "paths.cpp"
#include "bedtimes.cpp"
#include "taskscheduler.cpp"
#include "filewatching.cpp"

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
        // Default to watch bedtimes file for changes
        return WatchFile(paths.bedtimes);
    }

    return 0;
}