#include "ast.h"
#include <stdlib.h>
#include <string.h>

ASTNode *newNode(NodeType type)
{
    ASTNode *n = (ASTNode *)malloc(sizeof(ASTNode));
    if (!n)
        return NULL;
    memset(n, 0, sizeof(ASTNode));
    n->type = type;
    return n;
}

void freeNode(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_BINOP:
        freeNode(node->binop.left);
        freeNode(node->binop.right);
        break;
    case NODE_ASSIGN:
        freeNode(node->assign.value);
        break;
    case NODE_PRINT:
        if (node->print.exprs)
        {
            for (int i = 0; i < node->print.count; ++i)
                freeNode(node->print.exprs[i]); // FIX: access array element
            free(node->print.exprs);            // FIX: free the array, not call freeNode
        }
        break;
    case NODE_BLOCK:
        if (node->block.items)
        {
            for (int i = 0; i < node->block.count; ++i)
                freeNode(node->block.items[i]);
            free(node->block.items);
        }
        break;
    case NODE_IF:
        freeNode(node->ifstmt.cond);
        freeNode(node->ifstmt.thenBlock);
        freeNode(node->ifstmt.elseBlock);
        break;
    case NODE_FOR:
        freeNode(node->forstmt.init);
        freeNode(node->forstmt.cond);
        freeNode(node->forstmt.incr);
        freeNode(node->forstmt.body);
        break;
    case NODE_STR:
        if (node->string)
            free(node->string);
        break;
    default:
        break;
    }

    free(node);
}