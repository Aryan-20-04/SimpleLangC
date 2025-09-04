#include "symbol.h"
#include <stdio.h>   // for printf
#include <string.h>  // for strcmp, strncpy

SymEntry table[MAX_SYMBOLS];
int table_count = 0;

void setVar(const char *name, double value)
{
    for (int i = 0; i < table_count; ++i)
        if (strcmp(table[i].name, name) == 0)
        {
            table[i].value = value;
            return;
        }

    if (table_count < MAX_SYMBOLS)
    {
        strncpy(table[table_count].name, name, sizeof(table[table_count].name) - 1);
        table[table_count].name[sizeof(table[table_count].name) - 1] = '\0';
        table[table_count].value = value;
        table_count++;
    }
    else
    {
        printf("Error: symbol table full\n");
    }
}

double getVar(const char *name)
{
    for (int i = 0; i < table_count; ++i)
        if (strcmp(table[i].name, name) == 0)
            return table[i].value;
    return 0.0;
}

void clearSymbols(void)
{
    table_count = 0;
}