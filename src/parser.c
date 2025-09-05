#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "ast.h"
#include "lexer.h"

// Local pointer to source for tokenization (used only indirectly through getNextToken)
static const char *p_src = NULL;

// Forward declarations
static struct ASTNode *parseStatement(const char **p);
static struct ASTNode *parseBlock(const char **p);
static struct ASTNode *parseExpression(const char **p);
static struct ASTNode *parseComparison(const char **p);
static struct ASTNode *parseTerm(const char **p);
static struct ASTNode *parseFactor(const char **p);
static struct ASTNode *parseIfStatement(const char **p);
static struct ASTNode *parseFor(const char **p);
static struct ASTNode *parseAssignmentNoSemi(const char **p); // helper for for-header assignments

// Helper functions
static int expectTokenType(const char **p, TokenType t, const char *errMsg)
{
    Token tk = getNextToken(p);
    if (tk.type != t)
    {
        if (errMsg)
            printf("Syntax Error: %s (got '%s')\n", errMsg, tk.text);
        return 0;
    }
    return 1;
}

Token peekToken(const char **src)
{
    const char *save = *src;
    Token tk = getNextToken(src);
    *src = save; // reset position
    return tk;
}

int peekTokenType(const char **src)
{
    return peekToken(src).type;
}

// ----------------- Parsing Expressions -----------------

static struct ASTNode *parseFactor(const char **p)
{
    Token tk = getNextToken(p);

    if (tk.type == TOKEN_NUM)
    {
        struct ASTNode *n = newNode(NODE_NUM);
        n->number = strtod(tk.text, NULL);
        return n;
    }
    else if (tk.type == TOKEN_STR)
    {
        struct ASTNode *n = newNode(NODE_STR);
        n->string = strdup(tk.text);
        return n;
    }
    else if (tk.type == TOKEN_ID)
    {
        const char *save = *p;
        Token nxt = getNextToken(p);

        if (nxt.type == TOKEN_LPAREN)
        {
            struct ASTNode *fn = newNode(NODE_FUNC_CALL);
            fn->funcCall.funcName = strdup(tk.text);
            fn->funcCall.argCount = 0;
            fn->funcCall.args = NULL;

            if (peekTokenType(p) != TOKEN_RPAREN)
            {
                while (1)
                {
                    struct ASTNode *arg = parseComparison(p);
                    if (!arg)
                        break;
                    fn->funcCall.args = realloc(fn->funcCall.args,
                                                sizeof(struct ASTNode *) * (fn->funcCall.argCount + 1));
                    fn->funcCall.args[fn->funcCall.argCount++] = arg;

                    const char *saveComma = *p;
                    Token comma = getNextToken(p);
                    if (comma.type == TOKEN_COMMA)
                        continue;
                    else
                    {
                        *p = saveComma;
                        break;
                    }
                }
            }

            expectTokenType(p, TOKEN_RPAREN, "Expected ')' after function call");
            return fn;
        }
        else if (nxt.type == TOKEN_LBRACKET)
        {
            *p = save;
            struct ASTNode *varNode = newNode(NODE_VAR);
            strncpy(varNode->varName, tk.text, sizeof(varNode->varName) - 1);

            getNextToken(p); // consume '['
            struct ASTNode *idx = parseComparison(p);
            expectTokenType(p, TOKEN_RBRACKET, "Expected ']' after array index");

            struct ASTNode *acc = newNode(NODE_ARR_ACCESS);
            strncpy(acc->ArrAccessNode.varName, tk.text, sizeof(acc->ArrAccessNode.varName) - 1);
            acc->ArrAccessNode.index = idx;
            return acc;
        }
        else
        {
            *p = save;
            struct ASTNode *varNode = newNode(NODE_VAR);
            strncpy(varNode->varName, tk.text, sizeof(varNode->varName) - 1);
            return varNode;
        }
    }
    else if (tk.type == TOKEN_LPAREN)
    {
        struct ASTNode *e = parseComparison(p);
        expectTokenType(p, TOKEN_RPAREN, "Expected ')'");
        return e;
    }
    else if (tk.type == TOKEN_SUB)
    {
        struct ASTNode *f = parseFactor(p);
        struct ASTNode *zero = newNode(NODE_NUM);
        zero->number = 0.0;
        struct ASTNode *bin = newNode(NODE_BINOP);
        bin->binop.op = OP_SUB;
        bin->binop.left = zero;
        bin->binop.right = f;
        return bin;
    }
    else if (tk.type == TOKEN_PLUS)
    {
        return parseFactor(p);
    }
    else if (tk.type == TOKEN_LBRACKET) // array literal
    {
        struct ASTNode *arr = newNode(NODE_ARRAY);
        arr->ArrayNode.elements = NULL;
        arr->ArrayNode.count = 0;

        if (peekTokenType(p) != TOKEN_RBRACKET)
        {
            while (1)
            {
                struct ASTNode *elem = parseComparison(p);
                if (!elem)
                    break;

                arr->ArrayNode.elements = realloc(
                    arr->ArrayNode.elements,
                    sizeof(struct ASTNode *) * (arr->ArrayNode.count + 1));
                arr->ArrayNode.elements[arr->ArrayNode.count++] = elem;

                const char *save = *p;
                Token comma = getNextToken(p);
                if (comma.type == TOKEN_COMMA)
                    continue;
                else
                {
                    *p = save;
                    break;
                }
            }
        }

        expectTokenType(p, TOKEN_RBRACKET, "Expected ']' after array literal");
        return arr;
    }
    else
    {
        printf("Parser Error: Unexpected token '%s' in factor\n", tk.text);
        return NULL;
    }
}

static struct ASTNode *parseTerm(const char **p)
{
    struct ASTNode *left = parseFactor(p);
    if (!left)
        return NULL;

    while (1)
    {
        const char *save = *p;
        Token op = getNextToken(p);
        if (op.type == TOKEN_MUL || op.type == TOKEN_DIV)
        {
            struct ASTNode *right = parseFactor(p);
            struct ASTNode *bin = newNode(NODE_BINOP);
            bin->binop.left = left;
            bin->binop.right = right;
            bin->binop.op = (op.type == TOKEN_MUL) ? OP_MUL : OP_DIV;
            left = bin;
        }
        else
        {
            *p = save;
            break;
        }
    }
    return left;
}

static struct ASTNode *parseExpression(const char **p)
{
    struct ASTNode *left = parseTerm(p);
    if (!left)
        return NULL;

    while (1)
    {
        const char *save = *p;
        Token op = getNextToken(p);
        if (op.type == TOKEN_PLUS || op.type == TOKEN_SUB)
        {
            struct ASTNode *right = parseTerm(p);
            struct ASTNode *bin = newNode(NODE_BINOP);
            bin->binop.left = left;
            bin->binop.right = right;
            bin->binop.op = (op.type == TOKEN_PLUS) ? OP_ADD : OP_SUB;
            left = bin;
        }
        else
        {
            *p = save;
            break;
        }
    }
    return left;
}

static struct ASTNode *parseComparison(const char **p)
{
    struct ASTNode *left = parseExpression(p);
    if (!left)
        return NULL;

    const char *save = *p;
    Token op = getNextToken(p);
    BinOpType b;
    int isComp = 1;

    if (op.type == TOKEN_EQ)
        b = OP_EQ;
    else if (op.type == TOKEN_NE)
        b = OP_NE;
    else if (op.type == TOKEN_LT)
        b = OP_LT;
    else if (op.type == TOKEN_GT)
        b = OP_GT;
    else if (op.type == TOKEN_LE)
        b = OP_LE;
    else if (op.type == TOKEN_GE)
        b = OP_GE;
    else
        isComp = 0;

    if (!isComp)
    {
        *p = save;
        return left;
    }

    struct ASTNode *right = parseExpression(p);
    struct ASTNode *bin = newNode(NODE_BINOP);
    bin->binop.left = left;
    bin->binop.right = right;
    bin->binop.op = b;
    return bin;
}

// ----------------- Parsing Statements -----------------

static struct ASTNode *parseBlock(const char **p)
{
    if (!expectTokenType(p, TOKEN_LBRACE, "Expected '{' to start block"))
        return NULL;

    struct ASTNode *blk = newNode(NODE_BLOCK);
    blk->block.items = NULL;
    blk->block.count = 0;

    while (1)
    {
        const char *save = *p;
        Token tk = getNextToken(p);
        if (tk.type == TOKEN_RBRACE)
            break;
        if (tk.type == TOKEN_EOF)
        {
            printf("Parser Error: Unexpected EOF in block\n");
            break;
        }
        *p = save;
        struct ASTNode *stmt = parseStatement(p);
        if (stmt)
        {
            blk->block.items = realloc(blk->block.items,
                                       sizeof(struct ASTNode *) * (blk->block.count + 1));
            blk->block.items[blk->block.count++] = stmt;
        }
        else
        {
            Token skip = getNextToken(p);
            if (skip.type == TOKEN_EOF)
                break;
        }
    }
    return blk;
}

static struct ASTNode *parseIfStatement(const char **p)
{
    if (!expectTokenType(p, TOKEN_LPAREN, "Expected '(' after if"))
        return NULL;

    struct ASTNode *cond = parseComparison(p);
    if (!expectTokenType(p, TOKEN_RPAREN, "Expected ')' after if condition"))
        return NULL;

    struct ASTNode *thenBlk = parseBlock(p);
    struct ASTNode *elseBlk = NULL;

    const char *save = *p;
    Token next = getNextToken(p);
    if (next.type == TOKEN_ELSE)
    {
        const char *save2 = *p;
        Token maybeIf = getNextToken(p);
        if (maybeIf.type == TOKEN_IF)
        {
            *p = save2;
            elseBlk = parseIfStatement(p);
        }
        else
        {
            *p = save2;
            elseBlk = parseBlock(p);
        }
    }
    else if (next.type == TOKEN_ELSEIF)
    {
        elseBlk = parseIfStatement(p);
    }
    else
    {
        *p = save;
    }

    struct ASTNode *ifn = newNode(NODE_IF);
    ifn->ifstmt.cond = cond;
    ifn->ifstmt.thenBlock = thenBlk;
    ifn->ifstmt.elseBlock = elseBlk;
    return ifn;
}

// Parse an assignment or let-declaration without ';'
// Accepts:
//   let id = expr
//   id = expr
//   id[expr] = expr
static struct ASTNode *parseAssignmentNoSemi(const char **p)
{
    const char *save = *p;
    Token tk = getNextToken(p);

    // handle let x=...
    if (tk.type == TOKEN_LET)
    {
        Token id = getNextToken(p);
        if (id.type != TOKEN_ID)
        {
            printf("Syntax Error: Expected identifier after let\n");
            return NULL;
        }
        struct ASTNode *rhs = NULL;
        if (peekTokenType(p) == TOKEN_EQUAL)
        {
            getNextToken(p); // '='
            rhs = parseComparison(p);
        }
        struct ASTNode *decl = newNode(NODE_ASSIGN);
        strncpy(decl->assign.varName, id.text, sizeof(decl->assign.varName) - 1);
        decl->assign.value = rhs;
        return decl;
    }

    // not let → rewind and parse LHS
    *p = save;
    struct ASTNode *lhs = parseFactor(p); // could be var or arr[i]
    if (!lhs)
        return NULL;

    Token eq = getNextToken(p);
    if (eq.type != TOKEN_EQUAL)
    {
        *p = save;
        return NULL;
    }
    struct ASTNode *rhs = parseComparison(p);
    if (!rhs)
        return NULL;

    if (lhs->type == NODE_VAR)
    {
        struct ASTNode *stmt = newNode(NODE_ASSIGN);
        strncpy(stmt->assign.varName, lhs->varName, sizeof(stmt->assign.varName) - 1);
        stmt->assign.value = rhs;
        return stmt;
    }
    else if (lhs->type == NODE_ARR_ACCESS)
    {
        struct ASTNode *stmt = newNode(NODE_ARR_ASSIGN);
        strncpy(stmt->arrAssign.varName, lhs->ArrAccessNode.varName, sizeof(stmt->arrAssign.varName) - 1);
        stmt->arrAssign.index = lhs->ArrAccessNode.index;
        stmt->arrAssign.value = rhs;
        return stmt;
    }

    printf("Syntax Error: Invalid assignment target\n");
    return NULL;
}

static struct ASTNode *parseFor(const char **p)
{
    struct ASTNode *node = newNode(NODE_FOR);
    if (!expectTokenType(p, TOKEN_LPAREN, "Expected '(' after for"))
        return NULL;

    // init
    struct ASTNode *init = NULL;
    if (peekTokenType(p) != TOKEN_SEMI)
        init = parseAssignmentNoSemi(p);
    if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after for init"))
        return NULL;

    // condition
    struct ASTNode *cond = NULL;
    if (peekTokenType(p) != TOKEN_SEMI)
        cond = parseComparison(p);
    if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after for condition"))
        return NULL;

    // increment
    struct ASTNode *incr = NULL;
    if (peekTokenType(p) != TOKEN_RPAREN)
        incr = parseAssignmentNoSemi(p);
    if (!expectTokenType(p, TOKEN_RPAREN, "Expected ')' after for header"))
        return NULL;

    struct ASTNode *body = parseBlock(p);
    node->forstmt.init = init;
    node->forstmt.cond = cond;
    node->forstmt.incr = incr;
    node->forstmt.body = body;
    return node;
}

static struct ASTNode *parseStatement(const char **p)
{
    const char *save = *p;
    Token tk = getNextToken(p);

    if (tk.type == TOKEN_LET)
    {
        Token name = getNextToken(p);
        if (name.type != TOKEN_ID)
        {
            printf("Parser Error: Expected identifier after let\n");
            return NULL;
        }
        if (!expectTokenType(p, TOKEN_EQUAL, "Expected '=' after variable name"))
            return NULL;
        struct ASTNode *val = parseComparison(p);
        if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after assignment"))
            return NULL;

        struct ASTNode *asn = newNode(NODE_ASSIGN);
        strncpy(asn->assign.varName, name.text, sizeof(asn->assign.varName) - 1);
        asn->assign.value = val;
        return asn;
    }
    else if (tk.type == TOKEN_PRINT)
    {
        struct ASTNode *pn = newNode(NODE_PRINT);
        pn->print.count = 0;
        int cap = 8;
        pn->print.exprs = malloc(sizeof(struct ASTNode *) * cap);

        while (1)
        {
            struct ASTNode *expr = parseComparison(p);
            if (!expr)
                break;
            if (pn->print.count >= cap)
            {
                cap *= 2;
                pn->print.exprs = realloc(pn->print.exprs, sizeof(struct ASTNode *) * cap);
            }
            pn->print.exprs[pn->print.count++] = expr;

            const char *savec = *p;
            Token tkComma = getNextToken(p);
            if (tkComma.type != TOKEN_COMMA)
            {
                *p = savec;
                break;
            }
        }

        if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after print"))
            return NULL;

        return pn;
    }
    else if (tk.type == TOKEN_IF)
    {
        return parseIfStatement(p);
    }
    else if (tk.type == TOKEN_FOR)
    {
        return parseFor(p);
    }
    else if (tk.type == TOKEN_SEMI || tk.type == TOKEN_EOF)
    {
        return NULL;
    }

    // allow assignments and function-call statements starting with an identifier
    if (tk.type == TOKEN_ID)
    {
        *p = save; // rewind
        struct ASTNode *assignStmt = parseAssignmentNoSemi(p);
        if (assignStmt)
        {
            if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after assignment"))
                return NULL;
            return assignStmt;
        }

        // maybe it's a function call as a statement
        *p = save;
        struct ASTNode *possibleCall = parseFactor(p);
        if (possibleCall && possibleCall->type == NODE_FUNC_CALL)
        {
            if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after function call"))
                return NULL;
            return possibleCall;
        }

        // fall through to error
        *p = save;
    }

    printf("Parser Error: Unexpected token '%s' at statement start\n", tk.text);
    return NULL;
}

// ----------------- Top-Level -----------------

struct ASTNode *parseProgram(const char *src)
{
    p_src = src;
    const char *p = src;

    struct ASTNode *root = newNode(NODE_BLOCK);
    root->block.items = NULL;
    root->block.count = 0;

    while (1)
    {
        const char *save = p;
        Token tk = getNextToken(&p);
        if (tk.type == TOKEN_EOF || tk.type == TOKEN_RBRACE)
            break;
        p = save;
        struct ASTNode *stmt = parseStatement(&p);
        if (stmt)
        {
            root->block.items = realloc(root->block.items,
                                        sizeof(struct ASTNode *) * (root->block.count + 1));
            root->block.items[root->block.count++] = stmt;
        }
        else
        {
            Token t2 = getNextToken(&p);
            if (t2.type == TOKEN_EOF)
                break;
        }
    }
    return root;
}
