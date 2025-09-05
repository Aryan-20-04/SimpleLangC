#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

void execAST(struct ASTNode *node);
double evalExpr(struct ASTNode *node);

// buffered output functions
void flushOutput(void);

#endif
