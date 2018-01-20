#include "godnatt.h"

#include "common.cpp"
#include "bedtimes.cpp"

int main(int argc, char *argv[])
{
    Unused(argc);
    Unused(argv);

    char *Bedtimes = ReadEntireFile("bedtimes.txt");
    
    char *ParsedTimes[WeekdayCount] = {};
    bool succeeded = ParseBedtimes(Bedtimes, ParsedTimes);
    if (succeeded)
    {
        printf("Successfully parsed bedtimes.\n");
    }

    ShowError("ShowError %s\n", "is working.");

    FreeStringsInArray(ParsedTimes, WeekdayCount);
    free(Bedtimes);
    return 0;
}