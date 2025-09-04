#ifndef SYMBOL_H
#define SYMBOL_H

#include <string.h> // for strncpy
#include <stdio.h>  // for printf

#define MAX_SYMBOLS 1024

typedef struct
{
    char name[32]; // fixed-size, avoids strdup
    double value;
} SymEntry;

// only declare, do NOT define static variables/functions in header
extern SymEntry table[MAX_SYMBOLS];
extern int table_count;

// function prototypes
void setVar(const char *name, double value);
double getVar(const char *name);
void clearSymbols(void);

#endif
