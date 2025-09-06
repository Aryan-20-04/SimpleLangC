#include "symbol.h"
#include "ast.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

SymEntry table[MAX_SYMBOLS];
int table_count = 0;

SymEntry *lookupSym(const char *name);

/* -------------------- INTERNAL HELPERS -------------------- */

static int findIndex(const char *name)
{
    for (int i = 0; i < table_count; ++i)
        if (strcmp(table[i].name, name) == 0)
            return i;
    return -1;
}

static void freeEntryInternal(SymEntry *e)
{
    if (!e)
        return;
    if (e->type == SYM_ARRAY && e->v.arr.data)
    {
        free(e->v.arr.data);
        e->v.arr.data = NULL;
        e->v.arr.len = 0;
    }
}

/* resolveArray: follows array references */
static SymEntry *resolveArray(const char *name)
{
    SymEntry *sym = lookupSym(name);
    if (!sym)
        return NULL;

    if (sym->type == SYM_ARRAY_REF)
        return lookupSym(sym->v.arrRefName);

    return sym;
}

/* -------------------- SYMBOL TABLE OPERATIONS -------------------- */

void popSymbolsTo(int new_count)
{
    if (new_count < 0)
        new_count = 0;
    if (new_count >= table_count)
        return;

    for (int i = new_count; i < table_count; ++i)
    {
        freeEntryInternal(&table[i]);
        table[i].name[0] = '\0';
        table[i].type = 0;
    }
    table_count = new_count;
}

void setVar(const char *name, double value)
{
    int idx = findIndex(name);
    if (idx >= 0)
    {
        freeEntryInternal(&table[idx]);
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
    e->type = SYM_NUM;
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->name[sizeof(e->name) - 1] = '\0';
    e->v.num = value;
}

/* Always append new numeric symbol (local) */
void setVarLocal(const char *name, double value)
{
    if (table_count >= MAX_SYMBOLS)
    {
        printf("Error: symbol table full\n");
        return;
    }
    SymEntry *e = &table[table_count++];
    e->type = SYM_NUM;
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->name[sizeof(e->name) - 1] = '\0';
    e->v.num = value;
}

/* Append a local array reference */
void setVarLocalArrayRef(const char *localName, const char *existingArrayName)
{
    if (table_count >= MAX_SYMBOLS)
    {
        printf("Error: symbol table full\n");
        return;
    }
    SymEntry *e = &table[table_count++];
    e->type = SYM_ARRAY_REF;
    strncpy(e->name, localName, sizeof(e->name) - 1);
    e->name[sizeof(e->name) - 1] = '\0';
    e->v.arrRefName = existingArrayName;
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
    return sym && (sym->type == SYM_ARRAY || sym->type == SYM_ARRAY_REF);
}

int getArrayLen(const char *name)
{
    SymEntry *sym = resolveArray(name);
    if (!sym || sym->type != SYM_ARRAY)
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
        freeEntryInternal(&table[idx]);
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
    SymEntry *sym = resolveArray(name);
    if (!sym)
    {
        printf("Error: array '%s' not found\n", name);
        return 0;
    }
    if (sym->type != SYM_ARRAY)
    {
        printf("Type Error: '%s' is not an array\n", name);
        return 0;
    }
    if (idx < 0 || idx >= sym->v.arr.len)
    {
        printf("Index Error: '%s[%d]' out of bounds (len=%d)\n",
               name, idx, sym->v.arr.len);
        return 0;
    }
    *out = sym->v.arr.data[idx];
    return 1;
}

int setArrayAt(const char *name, int index, double value)
{
    SymEntry *sym = resolveArray(name);
    if (!sym)
    {
        printf("Error: array '%s' not found\n", name);
        return 0;
    }
    if (sym->type != SYM_ARRAY)
    {
        printf("Type Error: '%s' is not an array\n", name);
        return 0;
    }
    if (index < 0 || index >= sym->v.arr.len)
    {
        printf("Index Error: '%s[%d]' out of bounds (len=%d)\n",
               name, index, sym->v.arr.len);
        return 0;
    }
    sym->v.arr.data[index] = value;
    return 1;
}

void setFunc(const char *name, struct ASTNode *def)
{
    int idx = findIndex(name);
    if (idx >= 0)
    {
        table[idx].type = SYM_FUNC;
        table[idx].v.func.def = def;
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
    e->type = SYM_FUNC;
    e->v.func.def = def;
}

struct ASTNode *getFunc(const char *name)
{
    int idx = findIndex(name);
    if (idx < 0 || table[idx].type != SYM_FUNC)
        return NULL;
    return table[idx].v.func.def;
}

void clearSymbols(void)
{
    for (int i = 0; i < table_count; ++i)
        freeEntryInternal(&table[i]);
    table_count = 0;
}
