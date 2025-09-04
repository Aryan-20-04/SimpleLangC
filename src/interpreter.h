#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

void execStatement(const char **src);
int evalExpr(Token *tk, const char **src);

#endif