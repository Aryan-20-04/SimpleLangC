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
    NODE_STR,
    NODE_ARRAY,
    NODE_ARR_ACCESS,
    NODE_ARR_ASSIGN,
    NODE_FUNC_CALL,
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
        char *string;     // NODE_STR
        char varName[64]; // NODE_VAR

        struct
        {
            BinOpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;

        struct
        {
            char varName[64];
            struct ASTNode *value;
        } assign;

        struct
        {
            struct ASTNode **items;
            int count;
        } block;

        struct
        {
            struct ASTNode *cond;
            struct ASTNode *thenBlock;
            struct ASTNode *elseBlock;
        } ifstmt;

        struct
        {
            struct ASTNode *init;
            struct ASTNode *cond;
            struct ASTNode *incr;
            struct ASTNode *body;
        } forstmt;

        struct
        {
            struct ASTNode **exprs;
            int count;
        } print;

        struct
        {
            struct ASTNode **elements;
            int count;
        } ArrayNode;
        struct
        {
            char varName[32];
            struct ASTNode *index;
            struct ASTNode *value;
        } arrAssign;

        struct
        {
            char varName[32];
            struct ASTNode *index;
        } ArrAccessNode;
        
        struct
        {
            char *funcName;
            struct ASTNode **args;
            int argCount;
        } funcCall;
    };
} ASTNode; // ✅ keep only the typedef name here

// constructors / destructors
struct ASTNode *newNode(NodeType type);
void freeNode(struct ASTNode *node);

#endif
