#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

// parse the entire source and return an AST block node (root)
ASTNode *parseProgram(const char *src);

#endif
