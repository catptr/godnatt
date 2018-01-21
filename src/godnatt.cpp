#include "godnatt.h"

#include "common.cpp"
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
    }
    else
    {
        TaskScheduler taskScheduler;
        if (taskScheduler.did_succeed())
        {
            taskScheduler.add_weekly_trigger("2018-01-21T15:31:30", 6, "--lock");

            if (taskScheduler.did_succeed())
            {
                return 0;
            }
        }

        return 3;
        /*
        // Default to watch bedtimes file for changes
        const char *Path = "G:\\godnatt\\bedtimes.txt";
        return WatchFile(Path);
        */
    }

    return 0;
}