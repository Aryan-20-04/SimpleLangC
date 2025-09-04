#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

void execStatement(const char **src);
void execBlock(const char **src);
double parseExpression(const char **src);
double parseComparison(const char **src);
double parseTerm(const char **src);
double parseFactor(const char **src);
void printExpression(const char **src);

#endif