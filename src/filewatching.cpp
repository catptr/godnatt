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

const int DateTimeStringNumBytes = 20;

bool date_time_string(const char *bedtime, char result[])
{
    time_t t = time(0);

    struct tm now;
    if (localtime_s(&now, &t) != 0)
    {
        return false;
    }

    sprintf_s(result, DateTimeStringNumBytes, "%04d-%02d-%02dT%s:00", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, bedtime);

    return true;
}

bool date_time_string(const char *bedtime, char result[], int minuteOffset)
{
    time_t t = time(0);

    struct tm now;
    if (localtime_s(&now, &t) != 0)
    {
        return false;
    }

    sprintf_s(result, DateTimeStringNumBytes, "%04d-%02d-%02dT", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
    // 2018-01-21THH:MM:SS
    // Hours
    result[11] = bedtime[0];
    result[12] = bedtime[1];
    

    result[13] = ':';

    // Minutes
    int minutes = (bedtime[3] - '0') * 10 + (bedtime[4] - '0');
    minutes += minuteOffset;

    result[14] = (char)(minutes / 10) + '0';
    result[15] = (char)(minutes % 10) + '0';

    result[16] = ':';

    // Seconds
    result[17] = '0';
    result[18] = '0';

    result[19] = '\0';

    return true;
}

int WatchFile(const char *Directory, const char *Path)
{
    HANDLE ChangeHandle = FindFirstChangeNotificationA(Directory, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
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

                TaskScheduler taskScheduler;
                if (taskScheduler.did_succeed())
                {
                    char dateTimeString[DateTimeStringNumBytes] = {};
                    for (short i = 0; i < WeekdayCount; i++)
                    {
                        if (!date_time_string(ParsedTimes[i], dateTimeString))
                        {
                            ShowError("[ERROR::File Watching] date_time_string couldn't get localtime\n");
                            return 3;
                        }

                        taskScheduler.add_weekly_trigger(dateTimeString, i, "--lock");

                        if (!taskScheduler.did_succeed())
                        {
                            return 3;
                        }

                        if (!date_time_string(ParsedTimes[i], dateTimeString, -5))
                        {
                            ShowError("[ERROR::File Watching] date_time_string couldn't get localtime\n");
                            return 3;
                        }

                        taskScheduler.add_weekly_trigger(dateTimeString, i, "--notify");

                        if (!taskScheduler.did_succeed())
                        {
                            return 3;
                        }
                    }
                }
                else
                {
                    return 3;
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