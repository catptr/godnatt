#pragma once

struct Paths
{
    const char *directory;
    const char *bedtimes;
    const char *lastUpdate;
    const char *executable;

    Paths();
};

extern Paths paths;