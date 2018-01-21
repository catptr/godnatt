#include "filewatching.h"

FILETIME GetLastWriteTime(const char *Path)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Path, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return LastWriteTime;
}

bool FileTimeEquals(FILETIME a, FILETIME b)
{
    if (a.dwLowDateTime != b.dwLowDateTime) return false;
    if (a.dwHighDateTime != b.dwHighDateTime) return false;
    return true;
}

int WatchFile(const char *Path)
{
    // This will try to call GetLastWriteTime and ReadEntireFile on Path, even if it is a directory
    char DirPath[512] = { '.', '\\', '\0' };
    const char *LastBackslash = strrchr(Path, '\\');
    if (LastBackslash)
    {
        ptrdiff_t Length = LastBackslash - Path;
        // Path starts with a backslash, but strncpy_s won't copy 0 characters
        if (Length == 0)
        {
            DirPath[0] = '\\';
            DirPath[1] = '\0';
        }
        else if (Length > 0)
        {
            // If there was anything before the file name, copy it to DirPath
            assert(Length < 512);
            if (strncpy_s(DirPath, 512, Path, (size_t)Length) != 0)
            {
                ShowError("[ERROR::File Watching] strncpy_s failed\n");
                return 1;
            }
        }
    }

    HANDLE ChangeHandle = FindFirstChangeNotificationA(DirPath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (ChangeHandle == INVALID_HANDLE_VALUE)
    {
        ShowError("[ERROR::File Watching] FindFirstChangeNotification function failed: %x\n", GetLastError());
        return 1;
    }

    FILETIME LastWriteTime = GetLastWriteTime(Path);

    while (true)
    {
        printf("Waiting for notification...\n");

        DWORD WaitStatus = WaitForSingleObject(ChangeHandle, INFINITE);
        if (WaitStatus != WAIT_OBJECT_0)
        {
            ShowError("[ERROR::File Watching] WaitForSingleObject didn't return what we expected... %x\n", GetLastError());
            return 1;
        }

        printf("Received notification ");

        Sleep(1000);

        FILETIME WriteTime = GetLastWriteTime(Path);

        if (!FileTimeEquals(LastWriteTime, WriteTime))
        {
            // Our file changed
            printf("and our file did change\n");

            char *Bedtimes = ReadEntireFile(Path);
    
            char *ParsedTimes[WeekdayCount] = {};
            bool succeeded = ParseBedtimes(Bedtimes, ParsedTimes);
            free(Bedtimes);
            if (succeeded)
            {
                printf("Successfully parsed bedtimes. Here they are:\n");

                for (int i = 0; i < WeekdayCount; i++)
                {
                    printf("%s\n", ParsedTimes[i]);
                }
            }

            FreeStringsInArray(ParsedTimes, WeekdayCount);
        }
        else
        {
            printf("but our file didn't change\n");
        }

        LastWriteTime = WriteTime;

        if (FindNextChangeNotification(ChangeHandle) == 0)
        {
            ShowError("[ERROR::File Watching] FindNextChangeNotification failed: %x\n", GetLastError());
            return 1;
        }
    }
}