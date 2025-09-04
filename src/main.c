#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lexer.h"
#include "interpreter.h"

#define MAX_SRC_SIZE 65536

int main()
{
    FILE *fp = fopen("programs/program.txt", "r");
    if (!fp)
    {
        perror("Failed to open the source file");
        return 1;
    }
    char *source = malloc(MAX_SRC_SIZE);
    if (!source)
    {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }
    size_t len = fread(source, 1, MAX_SRC_SIZE - 1, fp);
    source[len] = '\0';
    fclose(fp);
    for (size_t i = 0; i < len; i++)
    {
        if (source[i] == '\r')
            source[i] = '\n';
    }
    const char *ptr = source;
    while (*ptr)
    {
        while (*ptr && isspace(*ptr))
        {
            ptr++;
        }
        if (*ptr == '\0')
            break;

        execStatement(&ptr);
    }

    free(source);
    return 0;
}