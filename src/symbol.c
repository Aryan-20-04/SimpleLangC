#include <stdio.h>
#include <string.h>
#include "symbol.h"

typedef struct
{
    char name[64];
    double val;
} Variable;

#define MAX_VARS 100
static Variable vars[MAX_VARS];
static int varCount = 0;

void setVar(char *name, double val)
{

    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {

            vars[i].val = val;
            return;
        }
    }
    if (varCount < MAX_VARS)
    {
        strncpy(vars[varCount].name, name, sizeof(vars[varCount].name) - 1);
        vars[varCount].name[sizeof(vars[varCount].name) - 1] = '\0';
        vars[varCount].val = val;
        varCount++;
    }
    else
    {
        printf("Error: Too many variables\n");
    }
}

double getVar(const char *name)
{

    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {

            return vars[i].val;
        }
    }
    printf("Error: variable '%s' not found\n", name);
    return 0.0;
}