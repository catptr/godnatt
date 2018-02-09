#pragma once

struct Paths
{
    const char *directory;
    const char *bedtimes;
    const char *lastUpdate;

    Paths();
};

extern Paths paths;