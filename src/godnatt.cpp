#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define UNUSED(x) (void)(x)

char *ReadEntireFile(const char *Path)
{
    FILE *File = nullptr;
    errno_t Error = fopen_s(&File, Path, "rb");
    if (Error != 0)
    {
        return nullptr;
    }

    fseek(File, 0, SEEK_END);
    size_t Size = (size_t)ftell(File);
    fseek(File, 0, SEEK_SET);

    char *Contents = (char *)malloc(sizeof(char) * (Size + 1));
    size_t NumRead = fread(Contents, 1, Size, File);
    fclose(File);

    if (NumRead < Size)
    {
        free(Contents);
        return nullptr;
    }

    Contents[Size] = '\0';

    return Contents;
}

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    char *Bedtimes = ReadEntireFile("bedtimes.txt");
    printf("%s", Bedtimes);
    free(Bedtimes);
    
    return 0;
}