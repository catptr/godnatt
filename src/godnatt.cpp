#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define Unused(x) (void)(x)

enum weekday
{
    Weekday_Monday,
    Weekday_Tuesday,
    Weekday_Wednesday,
    Weekday_Thursday,
    Weekday_Friday,
    Weekday_Saturday,
    Weekday_Sunday,
    Weekday_Count,
};

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

struct parser
{
    const char *Source;
    char Current;
    size_t NextIndex;
    size_t Length;
    int Line;
};

char NextChar(parser *Parser)
{
    if (Parser->NextIndex >= Parser->Length)
    {
        Parser->Current = EOF;
    }
    else 
    {
        Parser->Current = Parser->Source[Parser->NextIndex];
        Parser->NextIndex++;
    }

    return Parser->Current;
}

bool IsNewline(char Char)
{
    return Char == '\r' || Char == '\n';
}

bool IsDigit(char Char)
{
    return Char >= '0' && Char <= '9';
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

bool ParseBedtimes(const char *Bedtimes, char *Result[])
{
    parser Parser = {};
    Parser.Source = Bedtimes;
    Parser.Current = '\0';
    Parser.NextIndex = 0;
    Parser.Length = strlen(Bedtimes);
    Parser.Line = 1;

    int CurrentDay = 0;

    bool ExpectNewline = false;

    char Char;
    while ((Char = NextChar(&Parser)) != EOF)
    {
        if (Char == '#')
        {
            // Skip comment
            while (!IsNewline(Parser.Current) && Parser.Current != EOF) NextChar(&Parser);
        }

        if (Char == '\n')
        {
            Parser.Line++;
            ExpectNewline = false;
            continue;
        }

        if (ExpectNewline && Char != '\r')
        {
            // We expected a newline but didn't get any.
            // This is an error.
            fprintf(stderr, "[ERROR::Parsing] Some garbage on the same row as a time entry. Remove it. Row: %d\n", Parser.Line);
            FreeStringsInArray(Result, Weekday_Count);
            return false;
        }

        if (IsDigit(Char))
        {
            if (CurrentDay == Weekday_Count)
            {
                fprintf(stderr, "[ERROR::Parsing] You have entered more than seven times. How long is your week? Row: %d\n", Parser.Line);
                FreeStringsInArray(Result, Weekday_Count);
                return false;
            }

            // Try to parse a time.
            char Time[5] = { Char };
            NextChar(&Parser);

            for (int i = 1; i < 5; i++, NextChar(&Parser))
            {
                if (i == 2)
                {
                    if (Parser.Current != ':')
                    {
                        fprintf(stderr, "[ERROR::Parsing] Valid times follow the format \"HH:MM\" but your entry lacked a colon in the right place. Row: %d\n", Parser.Line);
                        FreeStringsInArray(Result, Weekday_Count);
                        return false;
                    }
                }
                else if (!IsDigit(Parser.Current))
                {
                    fprintf(stderr, "[ERROR::Parsing] Valid times follow the format \"HH:MM\" but character that should've been a digit wasn't. Row: %d\n", Parser.Line);
                    FreeStringsInArray(Result, Weekday_Count);
                    return false;
                }
                

                Time[i] = Parser.Current;
            }

            // A bit hacky... but because of the way we have our our outer loop setup
            // the character after each time entry would be skipped otherwise.
            // Undoes the last NextChar to compensate for the NextChar in the outer loop
            // Keeps getting hackier.
            if (Parser.Current != EOF)
            {
                Parser.NextIndex--;
                Parser.Current = Parser.Source[Parser.NextIndex - 1];
            }

            Result[CurrentDay] = (char *)malloc(sizeof(char) * 6);
            memcpy(Result[CurrentDay], Time, 5);
            Result[CurrentDay][5] = '\0';

            CurrentDay++;

            ExpectNewline = true;
        }
    }

    if (CurrentDay < Weekday_Count)
    {
        fprintf(stderr, "[ERROR::Parsing] You haven't entered times for all days of the week.\n");
        FreeStringsInArray(Result, Weekday_Count);
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    Unused(argc);
    Unused(argv);

    char *Bedtimes = ReadEntireFile("bedtimes.txt");
    
    char *ParsedTimes[Weekday_Count] = {};
    bool succeeded = ParseBedtimes(Bedtimes, ParsedTimes);
    if (succeeded)
    {
        printf("Successfully parsed bedtimes.\n");
    }

    free(Bedtimes);
    return 0;
}