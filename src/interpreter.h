#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

void execAST(ASTNode *node);
double evalExpr(ASTNode *node);

// buffered output functions
void flushOutput(void);

#endif
