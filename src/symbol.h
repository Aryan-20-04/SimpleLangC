#ifndef SYMBOL_H
#define SYMBOL_H

#include <stddef.h>

#define MAX_SYMBOLS 1024

/* forward declare ASTNode so symbol.h doesn't require ast.h include */
struct ASTNode;

typedef enum
{
    SYM_NUM,
    SYM_ARRAY,
    SYM_FUNC,
    SYM_ARRAY_REF
} SymType;

typedef struct
{
    double *data;
    int len;
} SymArray;

typedef struct
{
    SymType type;
    char name[64];
    union
    {
        double num;
        struct
        {
            double *data;
            int len;
        } arr;
        struct
        {
            struct ASTNode *def;
        } func;
        const char *arrRefName; // for SYM_ARRAY_REF
    } v;
} SymEntry;

extern SymEntry table[MAX_SYMBOLS];
extern int table_count;

/* numeric variables */
void setVar(const char *name, double value);      // set or update global variable
double getVar(const char *name);                  // get numeric variable
void setVarLocal(const char *name, double value); // always append new local variable
void setLocalVar(const char *name, double value); // alias for setVarLocal

/* arrays */
void setArray(const char *name, const double *data, int len); // create/copy array
int getArrayElem(const char *name, int idx, double *out);     // read element
int setArrayAt(const char *name, int idx, double value);      // write element
int isArray(const char *name);                                // true if array or ref
int getArrayLen(const char *name);

/* array reference (for passing arrays by reference) */
void setVarLocalArrayRef(const char *localName, const char *existingArrayName);

/* functions */
void setFunc(const char *name, struct ASTNode *funcDef);
struct ASTNode *getFunc(const char *name);

/* symbol table management */
void popSymbolsTo(int new_count);
void clearSymbols(void);

#endif
