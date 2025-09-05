#ifndef SYMBOL_H
#define SYMBOL_H

#include <stddef.h>

#define MAX_SYMBOLS 1024

typedef enum
{
    SYM_NUM,
    SYM_ARRAY
} SymType;

typedef struct
{
    double *data;
    int len;
} SymArray;

typedef struct
{
    char name[32];
    SymType type;
    union
    {
        double num;
        SymArray arr;
    } v;
} SymEntry;

extern SymEntry table[MAX_SYMBOLS];
extern int table_count;

void setVar(const char *name, double value); // number
double getVar(const char *name);             // number (0.0 if not found / not number)

void setArray(const char *name, const double *data, int len); // array (copies data)
int getArrayElem(const char *name, int idx, double *out);     // 1 on success, 0 on error
int isArray(const char *name);
int getArrayLen(const char *name);
int setArrayAt(const char *name, int index, double value);
void clearSymbols(void); // frees arrays

#endif
