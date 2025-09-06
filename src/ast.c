#include "ast.h"
#include <stdlib.h>
#include <string.h>

struct ASTNode *newNode(NodeType type)
{
    struct ASTNode *n = (struct ASTNode *)malloc(sizeof(ASTNode));
    if (!n)
        return NULL;
    memset(n, 0, sizeof(ASTNode));
    n->type = type;
    return n;
}

void freeNode(struct ASTNode *node)
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
                freeNode(node->print.exprs[i]);
            free(node->print.exprs);
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
    case NODE_ARRAY:
        if (node->ArrayNode.elements)
        {
            for (int i = 0; i < node->ArrayNode.count; ++i)
                freeNode(node->ArrayNode.elements[i]);
            free(node->ArrayNode.elements);
        }
        break;
    case NODE_ARR_ACCESS:
        freeNode(node->ArrAccessNode.index);
        break;

    case NODE_ARR_ASSIGN:
        freeNode(node->arrAssign.index);
        freeNode(node->arrAssign.value);
        break;
    case NODE_FUNC_DEF:
        if (node->funcDef.params)
        {
            for (int i = 0; i < node->funcDef.paramCount; i++)
            {
                free(node->funcDef.params[i]);
            }
            free(node->funcDef.params);
        }
        freeNode(node->funcDef.body);
        break;
    case NODE_RETURN:
        freeNode(node->returnStmt.value);
        break;
    default:
        break;
    }

    free(node);
}
