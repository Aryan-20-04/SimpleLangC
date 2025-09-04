#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "ast.h"
#include "lexer.h"

// Local pointer to source for tokenization (used only indirectly through getNextToken)
static const char *p_src = NULL;

// helpers
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

// Forward declarations
static ASTNode *parseStatement(const char **p);
static ASTNode *parseBlock(const char **p);
static ASTNode *parseExpression(const char **p);
static ASTNode *parseComparison(const char **p);
static ASTNode *parseTerm(const char **p);
static ASTNode *parseFactor(const char **p);

// parse number or identifier or parenthesized expression
static ASTNode *parseFactor(const char **p)
{
    Token tk = getNextToken(p);
    if (tk.type == TOKEN_NUM)
    {
        ASTNode *n = newNode(NODE_NUM);
        n->number = strtod(tk.text, NULL);
        return n;
    }
    else if (tk.type == TOKEN_ID)
    {
        ASTNode *n = newNode(NODE_VAR);
        strncpy(n->varName, tk.text, sizeof(n->varName) - 1);
        return n;
    }
    else if (tk.type == TOKEN_LPAREN)
    {
        ASTNode *e = parseComparison(p);
        expectTokenType(p, TOKEN_RPAREN, "Expected ')'");
        return e;
    }
    else if (tk.type == TOKEN_SUB)
    {
        // unary -
        ASTNode *f = parseFactor(p);
        ASTNode *zero = newNode(NODE_NUM);
        zero->number = 0.0;
        ASTNode *bin = newNode(NODE_BINOP);
        bin->binop.op = OP_SUB;
        bin->binop.left = zero;
        bin->binop.right = f;
        return bin;
    }
    else if (tk.type == TOKEN_PLUS)
    {
        return parseFactor(p);
    }
    else if (tk.type == TOKEN_STR)
    {
        ASTNode *n = newNode(NODE_STR);
        n->string = strdup(tk.text);
        return n;
    }
    else
    {
        printf("Parser Error: Unexpected token '%s' in factor\n", tk.text);
        return NULL;
    }
}

static ASTNode *parseTerm(const char **p)
{
    ASTNode *left = parseFactor(p);
    if (!left)
        return NULL;
    while (1)
    {
        const char *save = *p;
        Token op = getNextToken(p);
        if (op.type == TOKEN_MUL || op.type == TOKEN_DIV)
        {
            ASTNode *right = parseFactor(p);
            ASTNode *bin = newNode(NODE_BINOP);
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

static ASTNode *parseExpression(const char **p)
{
    ASTNode *left = parseTerm(p);
    if (!left)
        return NULL;
    while (1)
    {
        const char *save = *p;
        Token op = getNextToken(p);
        if (op.type == TOKEN_PLUS || op.type == TOKEN_SUB)
        {
            ASTNode *right = parseTerm(p);
            ASTNode *bin = newNode(NODE_BINOP);
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

static ASTNode *parseComparison(const char **p)
{
    ASTNode *left = parseExpression(p);
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

    ASTNode *right = parseExpression(p);
    ASTNode *bin = newNode(NODE_BINOP);
    bin->binop.left = left;
    bin->binop.right = right;
    bin->binop.op = b;
    return bin;
}

// parse a block { stmt* }
static ASTNode *parseBlock(const char **p)
{
    if (!expectTokenType(p, TOKEN_LBRACE, "Expected '{' to start block"))
        return NULL;
    ASTNode *blk = newNode(NODE_BLOCK);
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
        ASTNode *stmt = parseStatement(p);
        if (stmt)
        {
            blk->block.items = realloc(blk->block.items, sizeof(ASTNode *) * (blk->block.count + 1));
            blk->block.items[blk->block.count++] = stmt;
        }
        else
        {
            // on parse error, attempt to continue by advancing a token
            Token skip = getNextToken(p);
            if (skip.type == TOKEN_EOF)
                break;
        }
    }
    return blk;
}

ASTNode *parseFor(const char **src)
{
    ASTNode *node = newNode(NODE_FOR);

    expectTokenType(src, TOKEN_LPAREN, "Expected '(' after for");

    // init
    node->forstmt.init = parseStatement(src);

    // condition
    node->forstmt.cond = parseExpression(src);
    expectTokenType(src, TOKEN_SEMI, "Expected ';' after for condition");

    // increment
    node->forstmt.incr = parseStatement(src);
    expectTokenType(src, TOKEN_RPAREN, "Expected ')' after for header");

    // body
    node->forstmt.body = parseBlock(src);

    return node;
}

// FIXED: Simplified elseif parsing
static ASTNode *parseIfStatement(const char **p)
{
    if (!expectTokenType(p, TOKEN_LPAREN, "Expected '(' after if"))
        return NULL;

    ASTNode *cond = parseComparison(p);
    if (!expectTokenType(p, TOKEN_RPAREN, "Expected ')' after if condition"))
        return NULL;

    ASTNode *thenBlk = parseBlock(p);
    ASTNode *elseBlk = NULL;

    // Check for else/elseif
    const char *save = *p;
    Token next = getNextToken(p);

    if (next.type == TOKEN_ELSE)
    {
        // Check if it's followed by if (elseif case)
        const char *save2 = *p;
        Token maybeIf = getNextToken(p);

        if (maybeIf.type == TOKEN_IF)
        {
            // This is an "else if" - parse it as a new if statement
            *p = save2;                    // Reset to position after "else"
            elseBlk = parseIfStatement(p); // Recursive call to handle "if"
        }
        else
        {
            // This is a plain "else" - parse the block
            *p = save2; // Reset to position after "else"
            elseBlk = parseBlock(p);
        }
    }
    else if (next.type == TOKEN_ELSEIF)
    {
        // Direct elseif token
        elseBlk = parseIfStatement(p); // Parse as new if statement
    }
    else
    {
        // No else/elseif, rewind
        *p = save;
    }

    ASTNode *ifn = newNode(NODE_IF);
    ifn->ifstmt.cond = cond;
    ifn->ifstmt.thenBlock = thenBlk;
    ifn->ifstmt.elseBlock = elseBlk;
    return ifn;
}

// parse a single statement
static ASTNode *parseStatement(const char **p)
{
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
        ASTNode *val = parseComparison(p);
        if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after assignment"))
            return NULL;
        ASTNode *asn = newNode(NODE_ASSIGN);
        strncpy(asn->assign.varName, name.text, sizeof(asn->assign.varName) - 1);
        asn->assign.value = val;
        return asn;
    }
    else if (tk.type == TOKEN_PRINT)
    {
        ASTNode *pn = newNode(NODE_PRINT);
        pn->print.count = 0;
        // preallocate 8 expressions, can realloc if needed
        int cap = 8;
        pn->print.exprs = malloc(sizeof(ASTNode *) * cap);
        while (1)
        {
            ASTNode *expr = parseComparison(p);
            if (!expr)
                break;
            if (pn->print.count >= cap)
            {
                cap *= 2;
                pn->print.exprs = realloc(pn->print.exprs, sizeof(ASTNode *) * cap);
            }
            pn->print.exprs[pn->print.count++] = expr;

            const char *save = *p;
            Token tkComma = getNextToken(p);
            if (tkComma.type != TOKEN_COMMA)
            {
                *p = save;
                break;
            }
        }

        if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after print"))
            return NULL;

        return pn;
    }
    else if (tk.type == TOKEN_IF)
    {
        // Put back the IF token and call parseIfStatement
        // We need to move back to before the IF token
        // This is a bit tricky with our current setup, so let's handle it differently
        return parseIfStatement(p);
    }
    else if (tk.type == TOKEN_FOR)
    {
        if (!expectTokenType(p, TOKEN_LPAREN, "Expected '(' after for"))
            return NULL;

        // Lookahead to decide between "i in range(...)" or c-style init;cond;incr
        const char *look = *p;
        Token a = getNextToken(&look);
        Token b = getNextToken(&look);

        if (a.type == TOKEN_ID && b.type == TOKEN_IN)
        {
            // python-style: for (i in range(start,end[,step]))
            Token varTk = getNextToken(p); // var
            getNextToken(p);               // consume IN
            Token rangeTk = getNextToken(p);
            if (rangeTk.type != TOKEN_RANGE)
            {
                printf("Parser Error: expected 'range' after in\n");
                return NULL;
            }
            if (!expectTokenType(p, TOKEN_LPAREN, "Expected '(' after range"))
                return NULL;
            ASTNode *start = parseComparison(p);
            if (!expectTokenType(p, TOKEN_COMMA, "Expected ',' in range"))
                return NULL;
            ASTNode *end = parseComparison(p);
            ASTNode *step = NULL;
            const char *savep = *p;
            Token maybeComma = getNextToken(p);
            if (maybeComma.type == TOKEN_COMMA)
            {
                step = parseComparison(p);
            }
            else
            {
                *p = savep;
            }
            expectTokenType(p, TOKEN_RPAREN, "Expected ')' after range args");
            expectTokenType(p, TOKEN_RPAREN, "Expected ')' after for header");

            // Build equivalent C-style for nodes: init = var = start; cond = var < end; incr = var = var + step
            ASTNode *init = newNode(NODE_ASSIGN);
            strncpy(init->assign.varName, varTk.text, sizeof(init->assign.varName) - 1);
            init->assign.value = start;

            ASTNode *varNode = newNode(NODE_VAR);
            strncpy(varNode->varName, varTk.text, sizeof(varNode->varName) - 1);
            ASTNode *cond = newNode(NODE_BINOP);
            cond->binop.left = varNode;
            cond->binop.right = end;
            cond->binop.op = OP_LT;

            ASTNode *stepVal = step ? step : newNode(NODE_NUM);
            if (!step)
                stepVal->number = 1.0;
            ASTNode *vleft = newNode(NODE_VAR);
            strncpy(vleft->varName, varTk.text, sizeof(vleft->varName) - 1);
            ASTNode *add = newNode(NODE_BINOP);
            add->binop.left = vleft;
            add->binop.right = stepVal;
            add->binop.op = OP_ADD;
            ASTNode *incr = newNode(NODE_ASSIGN);
            strncpy(incr->assign.varName, varTk.text, sizeof(incr->assign.varName) - 1);
            incr->assign.value = add;

            ASTNode *body = parseBlock(p);
            ASTNode *forn = newNode(NODE_FOR);
            forn->forstmt.init = init;
            forn->forstmt.cond = cond;
            forn->forstmt.incr = incr;
            forn->forstmt.body = body;
            return forn;
        }
        else
        {
            // C-style: init ; cond ; incr
            const char *saveInit = *p;
            ASTNode *init = NULL;
            Token next = getNextToken(p);
            if (next.type == TOKEN_LET)
            {
                Token name = getNextToken(p);
                if (name.type != TOKEN_ID)
                {
                    printf("Parser Error: expected identifier in for init\n");
                    return NULL;
                }
                expectTokenType(p, TOKEN_EQUAL, "Expected '=' in for init");
                ASTNode *val = parseComparison(p);
                init = newNode(NODE_ASSIGN);
                strncpy(init->assign.varName, name.text, sizeof(init->assign.varName) - 1);
                init->assign.value = val;
            }
            else
            {
                // treat as possible assignment like i = 0
                *p = saveInit;
                ASTNode *maybeAssign = parseStatement(p);
                if (maybeAssign && maybeAssign->type == NODE_ASSIGN)
                    init = maybeAssign;
                else
                {
                    // if no init, allow empty (rewind)
                    *p = saveInit;
                }
            }

            if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after for init"))
                return NULL;

            // condition (could be empty -> true)
            const char *condPos = *p;
            ASTNode *cond = NULL;
            Token condTk = getNextToken(p);
            if (condTk.type == TOKEN_SEMI)
            {
                cond = NULL;
            }
            else
            {
                *p = condPos;
                cond = parseComparison(p);
                if (!expectTokenType(p, TOKEN_SEMI, "Expected ';' after for condition"))
                    return NULL;
            }

            // parse increment (assignment) or empty
            const char *tmp = *p;
            Token tchk = getNextToken(&tmp);
            if (tchk.type == TOKEN_RPAREN)
            {
                // empty incr -> consume ')' and parse body
                *p = tmp;
                expectTokenType(p, TOKEN_RPAREN, "Expected ')' after for header");
                ASTNode *body = parseBlock(p);
                ASTNode *forn = newNode(NODE_FOR);
                forn->forstmt.init = init;
                forn->forstmt.cond = cond;
                forn->forstmt.incr = NULL;
                forn->forstmt.body = body;
                return forn;
            }
            else
            {
                // attempt parse assignment: ID '=' expr
                const char *beforeInc = *p;
                Token id = getNextToken(p);
                if (id.type == TOKEN_ID)
                {
                    Token maybeEq = getNextToken(p);
                    if (maybeEq.type == TOKEN_EQUAL)
                    {
                        ASTNode *rhs = parseComparison(p);
                        ASTNode *incr = newNode(NODE_ASSIGN);
                        strncpy(incr->assign.varName, id.text, sizeof(incr->assign.varName) - 1);
                        incr->assign.value = rhs;
                        expectTokenType(p, TOKEN_RPAREN, "Expected ')' after for increment");
                        ASTNode *body = parseBlock(p);
                        ASTNode *forn = newNode(NODE_FOR);
                        forn->forstmt.init = init;
                        forn->forstmt.cond = cond;
                        forn->forstmt.incr = incr;
                        forn->forstmt.body = body;
                        return forn;
                    }
                    else
                    {
                        // not an assignment → require closing ')', then treat incr as NULL
                        *p = beforeInc;
                        if (!expectTokenType(p, TOKEN_RPAREN, "Expected ')' after for header"))
                            return NULL;
                        ASTNode *body = parseBlock(p);
                        ASTNode *forn = newNode(NODE_FOR);
                        forn->forstmt.init = init;
                        forn->forstmt.cond = cond;
                        forn->forstmt.incr = NULL;
                        forn->forstmt.body = body;
                        return forn;
                    }
                }
                else
                {
                    printf("Parser Error: Unexpected token in for increment\n");
                    return NULL;
                }
            }
        }
    }
    else if (tk.type == TOKEN_SEMI)
    {
        // empty statement
        return NULL;
    }
    else if (tk.type == TOKEN_EOF)
    {
        return NULL;
    }
    else
    {
        printf("Parser Error: Unexpected token '%s' at statement start\n", tk.text);
        return NULL;
    }
}

// Top-level parse: produce a BLOCK node containing statements
ASTNode *parseProgram(const char *src)
{
    p_src = src;
    const char *p = src;
    ASTNode *root = newNode(NODE_BLOCK);
    root->block.items = NULL;
    root->block.count = 0;

    while (1)
    {
        const char *save = p;
        Token tk = getNextToken(&p);
        if (tk.type == TOKEN_EOF || tk.type == TOKEN_RBRACE)
            break;
        p = save;
        ASTNode *stmt = parseStatement(&p);
        if (stmt)
        {
            root->block.items = realloc(root->block.items, sizeof(ASTNode *) * (root->block.count + 1));
            root->block.items[root->block.count++] = stmt;
        }
        else
        {
            // attempt to advance to avoid infinite loop
            Token t2 = getNextToken(&p);
            if (t2.type == TOKEN_EOF)
                break;
        }
    }

    return root;
}