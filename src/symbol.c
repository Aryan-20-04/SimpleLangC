#include "symbol.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

SymEntry table[MAX_SYMBOLS];
int table_count = 0;

static int findIndex(const char *name)
{
    for (int i = 0; i < table_count; ++i)
        if (strcmp(table[i].name, name) == 0)
            return i;
    return -1;
}

static void freeEntry(SymEntry *e)
{
    if (e->type == SYM_ARRAY && e->v.arr.data)
    {
        free(e->v.arr.data);
        e->v.arr.data = NULL;
        e->v.arr.len = 0;
    }
}

void setVar(const char *name, double value)
{
    int idx = findIndex(name);
    if (idx >= 0)
    {
        freeEntry(&table[idx]);
        table[idx].type = SYM_NUM;
        table[idx].v.num = value;
        return;
    }
    if (table_count >= MAX_SYMBOLS)
    {
        printf("Error: symbol table full\n");
        return;
    }
    SymEntry *e = &table[table_count++];
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->name[sizeof(e->name) - 1] = '\0';
    e->type = SYM_NUM;
    e->v.num = value;
}

double getVar(const char *name)
{
    int idx = findIndex(name);
    if (idx < 0)
    {
        printf("Error: variable '%s' not found\n", name);
        return 0.0;
    }
    if (table[idx].type != SYM_NUM)
    {
        printf("Type Error: '%s' is not a number\n", name);
        return 0.0;
    }
    return table[idx].v.num;
}

SymEntry *lookupSym(const char *name)
{
    int idx = findIndex(name);
    if (idx < 0)
        return NULL;
    return &table[idx];
}

int isArray(const char *name)
{
    SymEntry *sym = lookupSym(name);
    return sym && sym->type == SYM_ARRAY;
}

int getArrayLen(const char *name)
{
    SymEntry *sym = lookupSym(name);
    if (!sym)
        return 0;
    if (sym->type != SYM_ARRAY)
        return 0;
    return sym->v.arr.len;
}

void setArray(const char *name, const double *data, int len)
{
    int idx = findIndex(name);
    if (idx < 0)
    {
        if (table_count >= MAX_SYMBOLS)
        {
            printf("Error: symbol table full\n");
            return;
        }
        idx = table_count++;
        SymEntry *e = &table[idx];
        strncpy(e->name, name, sizeof(e->name) - 1);
        e->name[sizeof(e->name) - 1] = '\0';
        e->type = SYM_ARRAY;
        e->v.arr.data = NULL;
        e->v.arr.len = 0;
    }
    else
    {
        freeEntry(&table[idx]);
        table[idx].type = SYM_ARRAY;
    }

    if (len <= 0)
    {
        table[idx].v.arr.data = NULL;
        table[idx].v.arr.len = 0;
        return;
    }

    table[idx].v.arr.data = (double *)malloc(sizeof(double) * len);
    if (!table[idx].v.arr.data)
    {
        printf("Error: out of memory\n");
        table[idx].v.arr.len = 0;
        return;
    }
    memcpy(table[idx].v.arr.data, data, sizeof(double) * len);
    table[idx].v.arr.len = len;
}

int getArrayElem(const char *name, int idx, double *out)
{
    int i = findIndex(name);
    if (i < 0)
    {
        printf("Error: array '%s' not found\n", name);
        return 0;
    }
    if (table[i].type != SYM_ARRAY)
    {
        printf("Type Error: '%s' is not an array\n", name);
        return 0;
    }
    if (idx < 0 || idx >= table[i].v.arr.len)
    {
        printf("Index Error: '%s[%d]' out of bounds (len=%d)\n",
               name, idx, table[i].v.arr.len);
        return 0;
    }
    *out = table[i].v.arr.data[idx];
    return 1;
}

int setArrayAt(const char *name, int index, double value)
{
    int i = findIndex(name);
    if (i < 0)
    {
        printf("Error: array '%s' not found\n", name);
        return 0;
    }
    if (table[i].type != SYM_ARRAY)
    {
        printf("Type Error: '%s' is not an array\n", name);
        return 0;
    }
    if (index < 0 || index >= table[i].v.arr.len)
    {
        printf("Index Error: '%s[%d]' out of bounds (len=%d)\n",
               name, index, table[i].v.arr.len);
        return 0;
    }
    table[i].v.arr.data[index] = value;
    return 1;
}

void clearSymbols(void)
{
    for (int i = 0; i < table_count; ++i)
        freeEntry(&table[i]);
    table_count = 0;
}
