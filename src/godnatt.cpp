#include "godnatt.h"

#include "common.cpp"
#include "bedtimes.cpp"
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
        // Default to watch bedtimes file for changes
        const char *Path = "G:\\godnatt\\bedtimes.txt";
        return WatchFile(Path);
    }
}