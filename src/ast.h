#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef enum
{
    NODE_NUM,
    NODE_VAR,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_IF,
    NODE_FOR,
    NODE_STR
} NodeType;

typedef enum
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE
} BinOpType;

typedef struct ASTNode
{
    NodeType type;
    union
    {
        double number;    // NODE_NUM
        char *string;     // NODE_STR (allocated)
        char varName[64]; // NODE_VAR / NODE_ASSIGN

        // binary op
        struct
        {
            BinOpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;

        // assign
        struct
        {
            char varName[64];
            struct ASTNode *value;
        } assign;
        // block
        struct
        {
            struct ASTNode **items;
            int count;
        } block;

        // if
        struct
        {
            struct ASTNode *cond;
            struct ASTNode *thenBlock;
            struct ASTNode *elseBlock; // can be NULL
        } ifstmt;

        // for
        struct
        {
            struct ASTNode *init; // assignment or NULL
            struct ASTNode *cond; // expression or NULL (treated as true)
            struct ASTNode *incr; // assignment or NULL
            struct ASTNode *body; // block
            // For python-style for, init will be an assign to the loop var and incr is NULL; cond is comparison
        } forstmt;
        struct
        {
            struct ASTNode **exprs;
            int count;
        } print;
    };
} ASTNode;

// constructors / destructors
ASTNode *newNode(NodeType type);
void freeNode(ASTNode *node);

#endif
