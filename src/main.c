#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "interpreter.h"
#include "symbol.h"

#define MAX_SRC (1 << 20)

int main(int argc, char **argv)
{
    const char *fname = NULL;
    if (argc >= 2)
        fname = argv[1];

    const char *ext = strrchr(fname, '.');
    if (!ext || strcmp(ext, ".slc") != 0)
    {
        printf("Error: File must have .slc extension\n");
        return 1;
    }

    FILE *f = fopen(fname, "rb");
    if (!f)
    {
        perror("open");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len <= 0)
    {
        fclose(f);
        return 1;
    }
    char *src = malloc(len + 1);
    if (!src)
    {
        fclose(f);
        return 1;
    }
    fread(src, 1, len, f);
    src[len] = '\0';
    fclose(f);

    // parse program into AST
    struct ASTNode *program = parseProgram(src);
    if (!program)
    {
        printf("Parse failed\n");
        free(src);
        return 1;
    }
    execAST(program);
    flushOutput();

    // cleanup
    freeNode(program);
    clearSymbols();
    free(src);
    return 0;
}
