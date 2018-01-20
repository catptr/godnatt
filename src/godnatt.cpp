#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define UNUSED(x) (void)(x)

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    
#ifdef DEBUG
    printf("Hello, world!\n");
#else
    MessageBoxA(0, "Hello, world!", "godnatt", MB_OK | MB_ICONINFORMATION);
#endif
    
    return 0;
}