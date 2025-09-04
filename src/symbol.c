#include <stdio.h>
#include <string.h>
#include "symbol.h"

typedef struct
{
    char name[64];
    int val;
} Variable;

static Variable vars[100];
static int varCount = 0;

void setVar(char *name, int val)
{
    
    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            
            vars[i].val = val;
            return;
        }
    }
    strcpy(vars[varCount].name, name);
    vars[varCount].val = val;
    
    varCount++;
}

int getVar(char *name)
{
    
    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            
            return vars[i].val;
        }
    }
    printf("Error: variable '%s' not found\n", name);
    return 0;
}