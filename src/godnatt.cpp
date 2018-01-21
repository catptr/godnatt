#include "godnatt.h"

#include "common.cpp"
#include "bedtimes.cpp"
#include "filewatching.cpp"

int main(int argc, char *argv[])
{
    Unused(argc);
    Unused(argv);

    const char *Path = "G:\\godnatt\\bedtimes.txt";
    return WatchFile(Path);
}