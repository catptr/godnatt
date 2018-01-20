#include "common.h"

void ShowError(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);

#ifdef DEBUG
    vfprintf(stderr, Format, Args);
#else
    char Buf[256];
    vsnprintf_s(Buf, 256, Format, Args);
    MessageBoxA(0, Buf, "godnatt", MB_OK | MB_ICONERROR);
#endif
    
    va_end(Args);
}

void FreeStringsInArray(char *Array[], int Count)
{
    for (int i = 0; i < Count; i++)
    {
        if (Array[i])
        {
            free(Array[i]);
            Array[i] = nullptr;
        }
    }
}