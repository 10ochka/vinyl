
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "executor.h"
#include "object.h"
#include "strmap.h"
#include "common.h"
#include "string.h"


typedef struct Vnl_Token Vnl_Token;
typedef struct Vnl_Stack Vnl_Stack;

struct Vnl_Token {
    enum {
        TOK_IDENT,
        TOK_NUMLIT,
        TOK_STRLIT,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_LBRACK,
        TOK_RBRACK,
        TOK_COMMA,
        TOK_OP,
        TOK_EOF,
    } type;
    union {
        Vnl_String value;
        double numval;
    };
};

struct Vnl_Stack {
	Vnl_Object **stack;
	size_t len;
	size_t cap;
};

struct Vnl_Executor {
	Vnl_Stack stack;
    Vnl_StringMap *varlist;
    Vnl_Object *error;
};




static const Vnl_CString TOKTYPE_NAMES[] = {
    [TOK_IDENT]  = "Ident",
    [TOK_NUMLIT] = "Numlit",
    [TOK_STRLIT] = "Strlit",
    [TOK_LPAREN] = "LParen",
    [TOK_RPAREN] = "RParen",
    [TOK_LBRACK] = "LBrack",
    [TOK_RBRACK] = "RBrack",
    [TOK_COMMA]  = "Comma",
    [TOK_OP]     = "Operator",
    [TOK_EOF]    = "Eof"
};


typedef enum {
    LVALUE,
    RVALUE,
} ValueType;


typedef struct {
    Vnl_Token *items;
    size_t len;
    size_t cap;
} Tokens;


void tokens_push(Tokens *tokens, Vnl_Token token) {
    if (tokens->cap == tokens->len) {
        size_t newcap = tokens->cap;
        newcap = newcap ? newcap * 2 : 32;
        tokens->items = realloc(tokens->items, newcap * sizeof(Vnl_Token));
        tokens->cap = newcap;
    }
    tokens->items[tokens->len++] = token;
}

void tokens_free(Tokens *tokens) {
    free(tokens->items);
    tokens->items = nullptr;
    tokens->len = 0;
    tokens->cap = 0;
}

void token_display(const Vnl_Token *tok) {
    printf("Vnl_Token{");
    printf("type=%s", TOKTYPE_NAMES[tok->type]);
    switch (tok->type) {
        case TOK_NUMLIT:
            printf(", value=%g", tok->numval);
        break;

        case TOK_STRLIT:
            printf(", value=");
            vnl_string_print_escaped(tok->value);
        break;

        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_LBRACK:
        case TOK_RBRACK:
        case TOK_COMMA:
        case TOK_EOF:
        break;

        case TOK_IDENT:
        case TOK_OP:
            printf(", value=");
            vnl_string_print(tok->value);
        break;
    }
    printf("}");
}


typedef enum {
    PARSEERR_OK,
    PARSEERR_TOK_UNKNOWN,
    PARSEERR_TOK_NUMBER,
    PARSEERR_AST_UNFINISHED_STRLIT,

    PARSEERR_AST_EXPECTED_IDENT,
    PARSEERR_AST_EXPECTED_NUMLIT,
    PARSEERR_AST_EXPECTED_LPAREN,
    PARSEERR_AST_EXPECTED_RPAREN,
    PARSEERR_AST_EXPECTED_LBRACK,
    PARSEERR_AST_EXPECTED_RBRACK,
    PARSEERR_AST_EXPECTED_COMMA,
    PARSEERR_AST_EXPECTED_BINOP,
    PARSEERR_AST_EXPECTED_EOF,
    PARSEERR_AST_EXPECTED_LVALUE,
    PARSEERR_AST_EXPECTED_RVALUE,

    // PARSEERR_AST_UNEXPECTED_IDENT,
    // PARSEERR_AST_UNEXPECTED_NUMLIT,
    // PARSEERR_AST_UNEXPECTED_LPAREN,
    // PARSEERR_AST_UNEXPECTED_RPAREN,
    // PARSEERR_AST_UNEXPECTED_LBRACK,
    // PARSEERR_AST_UNEXPECTED_RBRACK,
    // PARSEERR_AST_UNEXPECTED_COMMA,
    // PARSEERR_AST_UNEXPECTED_BINOP,
    // PARSEERR_AST_UNEXPECTED_EOF,
    // PARSEERR_AST_UNEXPECTED_LVALUE,
    // PARSEERR_AST_UNEXPECTED_RVALUE,
} ParseError;


ParseError tokenize(Vnl_String *source, Tokens *tokens) {
    ParseError err = PARSEERR_OK;
    *source = vnl_string_ltrim(*source);

    while (source->len) {
        switch (source->chars[0]) {
            case '(': {
                *source = vnl_string_lshift(*source);
                Vnl_Token tok = { TOK_LPAREN, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case ')': {
                *source = vnl_string_lshift(*source);
                Vnl_Token tok = { TOK_RPAREN, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case '[': {
                *source = vnl_string_lshift(*source);
                Vnl_Token tok = { TOK_LBRACK, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case ']': {
                *source = vnl_string_lshift(*source);
                Vnl_Token tok = { TOK_RBRACK, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case ',': {
                *source = vnl_string_lshift(*source);
                Vnl_Token tok = { TOK_COMMA, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case '"': {
                *source = vnl_string_lshift(*source);
                Vnl_Token tok = { .type = TOK_STRLIT, .value = {source->chars, 0} };
                while(source->len && *source->chars != '"') {
                    *source = vnl_string_lshift(*source);
                    tok.value.len++;
                }
                if (!source->len) {
                    err = PARSEERR_AST_UNFINISHED_STRLIT;
                    goto return_err;
                }
                *source = vnl_string_lshift(*source);
                tokens_push(tokens, tok);
            } break;

            case '0' ... '9': {
                char *start, *end;
                start = end = (char *)source->chars;
                double numval = strtod(start, &end);
                if (start == end) {
                    err = PARSEERR_TOK_NUMBER;
                    goto return_err;
                }
                *source = vnl_string_lshiftn(*source, end - start);
                Vnl_Token tok = { TOK_NUMLIT, .numval = numval };
                tokens_push(tokens, tok);
            } break;

            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_': {
                Vnl_Token tok = { TOK_IDENT, .value = *source };
                tok.value.len = 0;
                while (source->len && (isalnum(*source->chars) || *source->chars == '_')) {
                    tok.value.len ++;
                    *source = vnl_string_lshift(*source);
                }
                tokens_push(tokens, tok);
            } break;

            case '+': case '-':
            case '*': case '/':
            case '%': case '=': {
                Vnl_Token tok = { TOK_OP, .value = { source->chars, 1 } };
                *source = vnl_string_lshift(*source);
                tokens_push(tokens, tok);
            } break;

            default:
                err = PARSEERR_TOK_UNKNOWN;
                goto return_err;
            break;

        } // switch (source.items[0])

        *source = vnl_string_ltrim(*source);
    } // while (source->len)

    return err;

    return_err:
        tokens_free(tokens);
        return err;
}



typedef enum {
    BINOP_SET,
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
} OpKind;

typedef struct {
    OpKind kind;
    Vnl_CString str;
    float left_bp;
    float right_bp;
} OpInfo;


OpInfo OPINFO_TABLE[] = {
    {BINOP_SET, "=", 1.0, 1.1},
    {BINOP_ADD, "+", 2.0, 2.1},
    {BINOP_SUB, "-", 2.0, 2.1},
    {BINOP_MUL, "*", 3.0, 3.1},
    {BINOP_DIV, "/", 3.0, 3.1},
    {BINOP_MOD, "%", 3.0, 3.1},
};


typedef enum {
    ASTTYPE_NUMLIT,
    ASTTYPE_IDENT,
    ASTTYPE_STRLIT,
    ASTTYPE_BINOP,
    ASTTYPE_CALL,
    ASTTYPE_ARRAY_LITERAL,
} ASTNodeType;



#define _ASTNODEBASE() struct { ASTNodeType _ast_type; ValueType _ast_valtype; }

typedef _ASTNODEBASE() ASTNode;



typedef struct {
    ASTNode *stmnts;
    size_t len;
    size_t cap;
} AST;

typedef struct {
    _ASTNODEBASE();
    double value;
} ASTNode_Numlit;

typedef struct {
    _ASTNODEBASE();
    Vnl_StringBuffer value;
} ASTNode_Ident;

typedef struct {
    _ASTNODEBASE();
    Vnl_StringBuffer value;
} ASTNode_Strlit;

typedef struct {
    _ASTNODEBASE();
    ASTNode *callee;
    ASTNode **args;
    size_t args_len;
    size_t args_cap;
} ASTNode_Call;


typedef struct {
    _ASTNODEBASE();
    ASTNode **items;
    size_t len;
    size_t cap;
} ASTNode_ArrayLiteral;

typedef struct {
    _ASTNODEBASE();
    OpKind op;
    ASTNode *lhs;
    ASTNode *rhs;
} ASTNode_BinOp;


typedef struct {
    const Tokens *tokens;
    size_t off;
} TokenIterator;


void arrlit_push(ASTNode_ArrayLiteral *arrlit, ASTNode *node) {
    if (arrlit->cap == arrlit->len) {
        size_t newcap = arrlit->cap;
        newcap = newcap ? newcap * 2 : 4;
        arrlit->items = realloc(arrlit->items, newcap * sizeof(ASTNode *));
        arrlit->cap = newcap;
    }
    arrlit->items[arrlit->len++] = node;
}

Vnl_Token titer_get(TokenIterator *titer) {
    if (titer->off < titer->tokens->len) {
        return titer->tokens->items[titer->off++];
    }
    return (Vnl_Token){ TOK_EOF };
}

Vnl_Token titer_peek(const TokenIterator *titer) {
    if (titer->off < titer->tokens->len) {
        return titer->tokens->items[titer->off];
    }
    return (Vnl_Token){ TOK_EOF };
}


void ast_free(ASTNode *node) {
    switch (node->_ast_type) {
            case ASTTYPE_NUMLIT: {
                free(node);
            } break;

            case ASTTYPE_STRLIT: {
                ASTNode_Strlit *strlit = (void *)node;
                vnl_strbuf_free(&strlit->value);
                free(node);
            } break;

            case ASTTYPE_IDENT: {
                ASTNode_Ident *ident = (ASTNode_Ident *)node;
                vnl_strbuf_free(&ident->value);
                free(ident);
            } break;

            case ASTTYPE_BINOP: {
                ASTNode_BinOp *binop = (void *)node;
                ast_free(binop->lhs);
                ast_free(binop->rhs);
                free(binop);
            } break;

            case ASTTYPE_CALL: {
                ASTNode_Call *call = (void *)node;
                ast_free(call->callee);
                for (size_t i = 0; i < call->args_len; ++i) {
                    ast_free(call->args[i]);
                }
                free(call);
            } break;

            case ASTTYPE_ARRAY_LITERAL: {
                ASTNode_ArrayLiteral *arr = (void *)node;
                for (size_t i = 0; i < arr->len; ++i) {
                    ast_free(arr->items[i]);
                }
                free(arr);
            } break;
    }
}


OpInfo get_op_info(Vnl_String strop) {
    const size_t numops = sizeof(OPINFO_TABLE)/sizeof(OPINFO_TABLE[0]);
    for (size_t i = 0; i < numops; ++i) {
        OpInfo opinfo = OPINFO_TABLE[i];
        if (vnl_string_cmpeq_c(strop, opinfo.str)) {
            return opinfo;
        }
    }
    printf(VNL_ANSICOL_RED "Invalid strop: %.*s\n" VNL_ANSICOL_RESET, (int)strop.len, strop.chars);
    abort();
}


ParseError parse_expression(TokenIterator *titer, ASTNode **expr, float min_bp);

ParseError parse_array_literal(TokenIterator *titer, ASTNode **node) {
    Vnl_Token tok;
    ParseError err = PARSEERR_OK;
    *node = nullptr;

    tok = titer_peek(titer);
    if (tok.type != TOK_LBRACK) {
        err = PARSEERR_AST_EXPECTED_LBRACK;
        goto return_failure;
    }
    titer_get(titer);

    ASTNode_ArrayLiteral *arr = vnl_malloc(sizeof(*arr));
    *arr = (ASTNode_ArrayLiteral){ { ASTTYPE_ARRAY_LITERAL, RVALUE } };

    while (true) {
        ASTNode *item = nullptr;
        err = parse_expression(titer, &item, 0.0);
        if (err) goto return_failure;
        arrlit_push(arr, item);
        item = nullptr;

        // expect ',' or ']'
        tok = titer_peek(titer);
        if (tok.type == TOK_COMMA) {
            titer_get(titer);
        } else if (tok.type == TOK_RBRACK) {
            titer_get(titer);
            goto return_success;
        } else {
            err = PARSEERR_AST_EXPECTED_LBRACK;
            goto return_failure;
        }
    }

    return_failure:
        if (arr) ast_free((ASTNode *)arr);
        if (*node) ast_free(*node);
        return err;

    return_success:
        *node = (ASTNode *)arr;
        return PARSEERR_OK;
}



ParseError parse_expression(TokenIterator *titer, ASTNode **expr, float min_bp) {
    Vnl_Token tok;
    *expr = nullptr;
    ASTNode *lhs = nullptr, *rhs = nullptr;
    ParseError err = PARSEERR_OK;

    tok = titer_peek(titer);
    switch (tok.type) {
        case TOK_RPAREN:
        case TOK_RBRACK:
        case TOK_COMMA:
        case TOK_OP:
        case TOK_EOF:
            err = PARSEERR_AST_EXPECTED_LVALUE;
            goto return_failure;

        case TOK_IDENT: {
            titer_get(titer);
            ASTNode_Ident *ident = vnl_malloc(sizeof(*ident));
            *ident = (ASTNode_Ident) { { ASTTYPE_IDENT, LVALUE }, (Vnl_StringBuffer){} };
            vnl_strbuf_append_s(&ident->value, tok.value);
            lhs = (void *)ident;
        } break;

        case TOK_STRLIT: {
            titer_get(titer);
            ASTNode_Strlit *strlit = vnl_malloc(sizeof(*strlit));
            *strlit = (ASTNode_Strlit){ { ASTTYPE_STRLIT, LVALUE }, (Vnl_StringBuffer){} };
            vnl_strbuf_append_s(&strlit->value, tok.value);
            lhs = (void *)strlit;
        } break;

        case TOK_NUMLIT: {
            titer_get(titer);
            ASTNode_Numlit *numlit = vnl_malloc(sizeof(*numlit));
            *numlit = (ASTNode_Numlit){ {ASTTYPE_NUMLIT, RVALUE }, tok.numval };
            lhs = (void *)numlit;
        } break;

        case TOK_LPAREN: {
            titer_get(titer);
            err = parse_expression(titer, &lhs, 0.0);
            if (err) goto return_failure;
            tok = titer_peek(titer);
            if (tok.type != TOK_RPAREN) {
                err = PARSEERR_AST_EXPECTED_RPAREN;
                goto return_failure;
            } else {
                titer_get(titer);
            }
        } break;


        case TOK_LBRACK: {
            err = parse_array_literal(titer, &lhs);
            if (err) goto return_failure;
        } break;
    } // switch (tok.type)


    while (true) {
        tok = titer_peek(titer);
        switch (tok.type) {
            case TOK_IDENT:
            case TOK_NUMLIT:
            case TOK_STRLIT:
            case TOK_LPAREN:
            case TOK_LBRACK:
                err = PARSEERR_AST_EXPECTED_BINOP;
                goto return_failure;

            case TOK_EOF:
            case TOK_RPAREN:
            case TOK_RBRACK:
            case TOK_COMMA:
                *expr = lhs;
                goto return_success;

            case TOK_OP:
            break;
        } // switch (tok.type)

        OpInfo opinfo = get_op_info(tok.value);

        if (opinfo.left_bp < min_bp) {
            *expr = lhs;
            goto return_success;
        }

        titer_get(titer);

        ASTNode *rhs = nullptr;
        err = parse_expression(titer, &rhs, opinfo.right_bp);
        if (err) goto return_failure;

        ASTNode_BinOp *binop = vnl_malloc(sizeof(*binop));
        *binop = (ASTNode_BinOp){ {ASTTYPE_BINOP, RVALUE}, opinfo.kind, lhs, rhs };
        lhs = (ASTNode *)binop;

    } // while (true)

    return_failure:
        if (lhs) ast_free(lhs);
        if (rhs) ast_free(rhs);
        return err;

    return_success:
        return PARSEERR_OK;

}


ParseError parse_statement(TokenIterator *titer, ASTNode **node) {
    *node = nullptr;
    ParseError err = parse_expression(titer, node, 0.0);
    if (err) return err;
    Vnl_Token tok = titer_peek(titer);
    if (tok.type != TOK_EOF) {
        if (*node) ast_free(*node);
        return PARSEERR_AST_EXPECTED_EOF;
    }
    return err;
}

void _ast_print_impl(const ASTNode *ast) {
    if (!ast) {
        printf("<nullptr>");
        return;
    }

    Vnl_CString valtype = ast->_ast_valtype == LVALUE ? "LValue" : "RValue";

    switch (ast->_ast_type) {
        case ASTTYPE_NUMLIT: {
            ASTNode_Numlit *numlit = (void *)ast;
            printf("Numlit(valtype=%s, value=%g)", valtype, numlit->value);
        } break;

        case ASTTYPE_IDENT: {
            ASTNode_Ident *ident = (void *)ast;
            printf("Ident(valtype=%s, value=%.*s)", valtype, (int)ident->value.len, ident->value.chars);
        } break;

        case ASTTYPE_STRLIT: {
            ASTNode_Strlit *strlit = (void *)ast;
            printf("Strlit(valtype=%s, value=%.*s)", valtype, (int)strlit->value.len, strlit->value.chars);
        } break;

        case ASTTYPE_BINOP: {
            ASTNode_BinOp *binop = (void *)ast;
            OpInfo opinfo = OPINFO_TABLE[binop->op];
            printf("Ident(valtype=%s, op='%s', lhs=", valtype, opinfo.str);
            _ast_print_impl(binop->lhs);
            printf(", rhs=");
            _ast_print_impl(binop->rhs);
            printf(")");
        } break;

        case ASTTYPE_CALL: {
            ASTNode_Call *call = (void *)ast;
            printf("Call(valtype=%s, callee=", valtype);
            _ast_print_impl(call->callee);
            printf(", args=[");
            for (size_t i = 0; i < call->args_len; ++i) {
                _ast_print_impl(call->args[i]);
                if (i < call->args_len - 1) {
                    printf(", ");
                } else {
                    printf("])");
                }
            }
        } break;

        case ASTTYPE_ARRAY_LITERAL: {
            ASTNode_ArrayLiteral *arrlit = (void *)ast;
            printf("ArrayLiteral(valtype=%s, items=[", valtype);
            for (size_t i = 0; i < arrlit->len; ++i) {
                _ast_print_impl(arrlit->items[i]);
                if (i < arrlit->len - 1) {
                    printf(", ");
                } else {
                    printf("])");
                }
            }
        } break;
    }
}

void ast_print(const ASTNode *ast) {
    _ast_print_impl(ast);
}

void ast_println(const ASTNode *ast) {
    _ast_print_impl(ast);
    printf("\n");
}


void print_parseerr(ParseError err, Vnl_String line, const ASTNode *ast, const TokenIterator *titer) {
    switch (err) {
        case PARSEERR_OK: {
            printf(VNL_ANSICOL_GREEN "Error: Success!\n" VNL_ANSICOL_RESET);
        } break;

        case PARSEERR_TOK_UNKNOWN: {
            printf(VNL_ANSICOL_RED "Error: Unknown character '%c' (%d)\n" VNL_ANSICOL_RESET, *line.chars, *line.chars);
        } break;

        case PARSEERR_TOK_NUMBER: {
            printf(VNL_ANSICOL_RED "Error: Cannot parse number from position: \"\n");
            if (line.len <= 10) {
                vnl_string_print(line);
                printf("\"\n"VNL_ANSICOL_RESET);
            } else {
                printf("%.*s" VNL_ANSICOL_CYAN "..." VNL_ANSICOL_RED "\"\n" VNL_ANSICOL_RESET, 5, line.chars);
            }
        } break;

        case PARSEERR_AST_UNFINISHED_STRLIT: {
            printf(VNL_ANSICOL_RED "Error: Unfinished vnl_string literal" VNL_ANSICOL_RESET);
        } break;

        case PARSEERR_AST_EXPECTED_IDENT: {
            printf(VNL_ANSICOL_RED "Error: Expected ident, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_NUMLIT: {
            printf(VNL_ANSICOL_RED "Error: Expected numlit, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_LPAREN: {
            printf(VNL_ANSICOL_RED "Error: Expected lparen, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_RPAREN: {
            printf(VNL_ANSICOL_RED "Error: Expected rparen, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_LBRACK: {
            printf(VNL_ANSICOL_RED "Error: Expected lbrack, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_RBRACK: {
            printf(VNL_ANSICOL_RED "Error: Expected rbrack, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_COMMA: {
            printf(VNL_ANSICOL_RED "Error: Expected comma, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_BINOP: {
            printf(VNL_ANSICOL_RED "Error: Expected binop, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_EOF: {
            printf(VNL_ANSICOL_RED "Error: Expected eof, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_LVALUE: {
            printf(VNL_ANSICOL_RED "Error: Expected lvalue, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_RVALUE: {
            printf(VNL_ANSICOL_RED "Error: Expected rvalue, got ");
            Vnl_Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

    }
}





typedef enum {
    EXEC_OK  = 0,
    EXEC_ERR = 1,
} ExecError;

typedef enum {
    VM_SET,
    VM_ADD,
    VM_SUB,
    VM_MUL,
    VM_DIV,
    VM_MOD,

    VM_LOAD,
    VM_STORE,

    VM_ROT,
    VM_DUP,
    VM_PUT,
    VM_MAKEARR,
} VMOpcode;

typedef struct {
    VMOpcode opcode;
    union {
        Vnl_Object *arg;
        Vnl_String varname;
        size_t makearr_len;
    };
} Instruction;


typedef struct {
    Instruction *items;
    size_t len;
    size_t cap;
} Code;



void code_append(Code *code, Instruction instr) {
    if (code->cap == code->len) {
        size_t newcap = code->cap;
        newcap = newcap ? newcap * 2 : 32;
        code->items = realloc(code->items, newcap * sizeof(Vnl_Token));
        code->cap = newcap;
    }
    code->items[code->len++] = instr;
}



void object_print(const Vnl_Object *obj) {
    switch (obj->type) {
        case VNL_OBJTYPE_NUMBER:
            printf("%g", ((Vnl_NumberObject*)obj)->value);
        break;

        case VNL_OBJTYPE_STRING: {
            Vnl_StringObject *strobj = (void *)obj;
            vnl_string_print_escaped(vnl_string_from_b(&strobj->value));
        } break;

        case VNL_OBJTYPE_ARRAY:
            printf("[");
            Vnl_ArrayObject *arr = (void *)obj;
            for (size_t i = 0; i < arr->len; ++i) {
                object_print(arr->items[i]);
                if (i < arr->len - 1)
                    printf(", ");
            }
            printf("]");
        break;
    }
}


void code_print(const Code *code) {
    for (size_t i = 0; i < code->len; ++i) {
        Instruction instr = code->items[i];
        switch (instr.opcode) {
            case VM_SET: {
                printf("SET\n");
            } break;

            case VM_ADD: {
                printf("ADD\n");
            } break;

            case VM_SUB: {
                printf("SUB\n");
            } break;

            case VM_MUL: {
                printf("MUL\n");
            } break;

            case VM_DIV: {
                printf("DIV\n");
            } break;

            case VM_MOD: {
                printf("MOD\n");
            } break;

            case VM_LOAD: {
                printf("LOAD ");
                vnl_string_println(instr.varname);
            } break;

            case VM_STORE: {
                printf("STORE ");
                vnl_string_println(instr.varname);
            } break;

            case VM_ROT: {
                printf("ROT\n");
            } break;

            case VM_DUP: {
                printf("DUP\n");
            } break;

            case VM_PUT: {
                printf("PUT ");
                object_print(instr.arg);
                printf("\n");
            } break;

            case VM_MAKEARR: {
                printf("MAKEARR %zu\n", instr.makearr_len);
            } break;

        }
    }
}



Vnl_NumberObject *exec_create_number(Vnl_Executor *exec, double value) {
    Vnl_NumberObject *obj = vnl_object_create(sizeof(*obj), VNL_OBJTYPE_NUMBER);
    obj->value = value;
    return obj;
}

void exec_stack_push(Vnl_Executor *exec, Vnl_Object *obj) {
	vnl_object_acquire(obj);
    if (exec->stack.cap == exec->stack.len) {
        size_t newcap = exec->stack.cap;
        newcap = newcap ? newcap * 2 : 32;
        exec->stack.stack = realloc(exec->stack.stack, newcap * sizeof(Vnl_Token));
        exec->stack.cap = newcap;
    }
    exec->stack.stack[exec->stack.len++] = obj;
}


Vnl_Object *exec_stack_pop(Vnl_Executor *exec) {
    if (exec->stack.len == 0) {
        return nullptr;
    } else {
        Vnl_Object *obj = exec->stack.stack[--exec->stack.len];
        return obj;
    }
}


void exec_set_error(Vnl_Executor *exec, Vnl_Object *err_obj) {
	vnl_object_release(err_obj);
    exec->error = err_obj;
    vnl_object_acquire(err_obj);
}



bool obj_is_number(Vnl_Object *obj) {
    return obj && obj->type == VNL_OBJTYPE_NUMBER;
}


bool isintegral(double x) {
    double i;
    return modf(x, &i) == 0.0;
}

bool obj_is_integer(Vnl_Object *obj) {
    return obj && obj->type == VNL_OBJTYPE_NUMBER && isintegral(((Vnl_NumberObject *)obj)->value);
}

bool obj_is_string(Vnl_Object *obj) {
    return obj && obj->type == VNL_OBJTYPE_STRING;
}

Vnl_CString objtype_as_str(Vnl_Object *obj) {
    static const Vnl_CString OBJTYPE2STR[] = {
        [VNL_OBJTYPE_NUMBER] = "number",
        [VNL_OBJTYPE_STRING] = "number",
        [VNL_OBJTYPE_ARRAY] = "array",
    };
    return OBJTYPE2STR[obj->type];
}

int64_t obj_as_integer(Vnl_Object *obj) {
    return ((Vnl_NumberObject *)obj)->value;
}

uint64_t obj_as_uinteger(Vnl_Object *obj) {
    return ((Vnl_NumberObject *)obj)->value;
}

void error_invalid_binop_args(OpKind op, Vnl_Object *a, Vnl_Object *b) {
    if (!a && !b) {
        printf(VNL_ANSICOL_RED "Error: not enough arguments, stack exceeded!\n" VNL_ANSICOL_RESET);
        exit(1);
    } else {
        OpInfo opinfo = OPINFO_TABLE[op];
        printf(
            VNL_ANSICOL_RED "Error: "
            "Unsupported arguments for operator '%s':"
            " <%s> and <%s>\n" VNL_ANSICOL_RESET,
            opinfo.str,
            objtype_as_str(a),
            objtype_as_str(b)
        );
    }
}


void obj_array_push(Vnl_ArrayObject *arr, Vnl_Object *obj) {
    if (arr->cap == arr->len) {
        size_t newcap = arr->cap;
        newcap = newcap ? newcap * 2 : 32;
        arr->items = realloc(arr->items, newcap * sizeof(Vnl_Token));
        arr->cap = newcap;
    }
    arr->items[arr->len++] = obj;
}


void obj_arr_reverse(Vnl_ArrayObject *arr) {
    for (size_t i = 0; i < arr->len / 2; ++i) {
        Vnl_Object *tmp = arr->items[i];
        arr->items[i] = arr->items[arr->len - 1 - i];
        arr->items[arr->len - 1 - i] = tmp;
    }
}



ExecError exec_code(Vnl_Executor *exec, const Code *code) {
    for (size_t pc = 0; pc < code->len; ++pc) {
        Instruction instr = code->items[pc];
        switch (instr.opcode) {
            case VM_SET: {
                printf(VNL_ANSICOL_RED "TODO: Illegal instruction\n" VNL_ANSICOL_RESET);
                exit(1);
            } break;

            case VM_ADD: {
                Vnl_Object *a = exec_stack_pop(exec);
                Vnl_Object *b = exec_stack_pop(exec);

                if (obj_is_number(a) && obj_is_number(b)) {
                    Vnl_NumberObject *anum = (void *)a;
                    Vnl_NumberObject *bnum = (void *)b;
                    Vnl_NumberObject *result = exec_create_number(exec, anum->value + bnum->value);
                    vnl_object_release(a);
                    vnl_object_release(b);
                    exec_stack_push(exec, (Vnl_Object *)result);
                } else if (obj_is_string(a) && obj_is_string(b)) {
                    Vnl_StringObject *astr = (void *)a;
                    Vnl_StringObject *bstr = (void *)b;
                    Vnl_StringObject *result = vnl_object_create(sizeof(*result), VNL_OBJTYPE_STRING);
                    vnl_strbuf_append_s(&result->value, vnl_string_from_b(&astr->value));
                    vnl_strbuf_append_s(&result->value, vnl_string_from_b(&bstr->value));
                    vnl_object_release(a);
                    vnl_object_release(b);
                    exec_stack_push(exec, (Vnl_Object *)result);
                } else {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }
            } break;

            case VM_SUB: {
                Vnl_Object *a = exec_stack_pop(exec);
                Vnl_Object *b = exec_stack_pop(exec);

                if (!obj_is_number(a) || !obj_is_number(b)) {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }

                Vnl_NumberObject *anum = (void *)a;
                Vnl_NumberObject *bnum = (void *)b;
                Vnl_NumberObject *result = exec_create_number(exec, anum->value - bnum->value);
                vnl_object_release(a);
                vnl_object_release(b);
                exec_stack_push(exec, (Vnl_Object *)result);
            } break;

            case VM_MUL: {
                Vnl_Object *a = exec_stack_pop(exec);
                Vnl_Object *b = exec_stack_pop(exec);

                if (obj_is_number(a) && obj_is_number(b)) {
                    Vnl_NumberObject *anum = (void *)a;
                    Vnl_NumberObject *bnum = (void *)b;
                    Vnl_NumberObject *result = exec_create_number(exec, anum->value * bnum->value);
                    vnl_object_release(a);
                    vnl_object_release(b);
                    exec_stack_push(exec, (Vnl_Object *)result);
                } else if (obj_is_integer(a) && obj_is_string(b) && obj_as_integer(a) >= 0) {
                    size_t times = obj_as_integer(a);
                    Vnl_StringObject *result = vnl_object_create(sizeof(*result), VNL_OBJTYPE_STRING);
                    Vnl_StringObject *bstr = (void *)b;
                    for (size_t i = 0; i < times; ++i) {
                        vnl_strbuf_append_s(&result->value, vnl_string_from_b(&bstr->value));
                    }
                    vnl_object_release(a);
                    vnl_object_release(b);
                    exec_stack_push(exec, (Vnl_Object *)result);
                } else if (obj_is_integer(b) && obj_is_string(a) && obj_as_integer(b) >= 0) {
                    size_t times = obj_as_integer(b);
                    Vnl_StringObject *result = vnl_object_create(sizeof(*result), VNL_OBJTYPE_STRING);
                    Vnl_StringObject *astr = (void *)a;
                    for (size_t i = 0; i < times; ++i) {
                        vnl_strbuf_append_s(&result->value, vnl_string_from_b(&astr->value));
                    }
                    vnl_object_release(a);
                    vnl_object_release(b);
                    exec_stack_push(exec, (Vnl_Object *)result);
                } else {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }
            } break;

            case VM_DIV: {
                Vnl_Object *a = exec_stack_pop(exec);
                Vnl_Object *b = exec_stack_pop(exec);

                if (!obj_is_number(a) || !obj_is_number(b)) {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }

                Vnl_NumberObject *anum = (void *)a;
                Vnl_NumberObject *bnum = (void *)b;
                Vnl_NumberObject *result = exec_create_number(exec, anum->value / bnum->value);
                vnl_object_release(a);
                vnl_object_release(b);
                exec_stack_push(exec, (Vnl_Object *)result);
            } break;

            case VM_MOD: {
                Vnl_Object *a = exec_stack_pop(exec);
                Vnl_Object *b = exec_stack_pop(exec);

                if (!obj_is_number(a) || !obj_is_number(b)) {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }

                Vnl_NumberObject *anum = (void *)a;
                Vnl_NumberObject *bnum = (void *)b;

                Vnl_NumberObject *result = exec_create_number(exec, fmod(anum->value, bnum->value));
                vnl_object_release(a);
                vnl_object_release(b);
                exec_stack_push(exec, (Vnl_Object *)result);
            } break;

            case VM_LOAD: {
                Vnl_String varname = instr.varname;
                Vnl_Object *obj = vnl_exec_getvar(exec, varname);
                if (!obj) {
                    printf(VNL_ANSICOL_RED "Error: Unknown variable: ");
                    vnl_string_println(varname);
                    printf(VNL_ANSICOL_RESET);
                    return EXEC_ERR;
                }
                exec_stack_push(exec, obj);
            } break;

            case VM_STORE: {
                Vnl_String varname = instr.varname;
                Vnl_Object *obj = exec_stack_pop(exec);
                if (!obj) {
                    printf(VNL_ANSICOL_RED "Error: No value to store - stack is empty!\n" VNL_ANSICOL_RESET);
                    return EXEC_ERR;
                }
                vnl_exec_setvar(exec, varname, obj);
            } break;

            case VM_ROT: {
               if (exec->stack.len < 2) {
                   printf(VNL_ANSICOL_RED "Error: not enough values to ROT!");
                   return EXEC_ERR;
               }
               Vnl_Object *tmp = exec->stack.stack[exec->stack.len - 1];
               exec->stack.stack[exec->stack.len - 1] = exec->stack.stack[exec->stack.len - 2];
               exec->stack.stack[exec->stack.len - 2] = tmp;
            } break;

            case VM_DUP: {
                if (!exec->stack.len) {
                    printf(VNL_ANSICOL_RED "Error: Cannot DUP - stack is empty!\n" VNL_ANSICOL_RESET);
                    return EXEC_ERR;
                } else {
                    exec_stack_push(exec, exec->stack.stack[exec->stack.len-1]);
                }
            } break;

            case VM_PUT: {
                exec_stack_push(exec, instr.arg);
            } break;

            case VM_MAKEARR: {
                size_t arrsize = instr.makearr_len;
                Vnl_ArrayObject *arr = vnl_object_create(sizeof(*arr), VNL_OBJTYPE_ARRAY);
                for (size_t i = 0; i < arrsize; ++i) {
                    Vnl_Object *obj = exec_stack_pop(exec);
                    obj_array_push(arr, obj);
                }
                obj_arr_reverse(arr);
                exec_stack_push(exec, (Vnl_Object *)arr);
            } break;

        }
    }
    return EXEC_OK;
}


void exec_compile_ast(Vnl_Executor *exec, const ASTNode *ast, Code *compile_result) {
    switch (ast->_ast_type) {
        case ASTTYPE_NUMLIT: {
            const ASTNode_Numlit *astnode = (void *)ast;
            Vnl_NumberObject *obj = exec_create_number(exec, astnode->value);
            Instruction instr = { VM_PUT, .arg = (Vnl_Object *)obj };
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_IDENT: {
            const ASTNode_Ident *astnode = (void *)ast;
            Vnl_String varname = vnl_string_from_b(&astnode->value);
            Instruction instr = { VM_LOAD, .varname = varname };
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_STRLIT: {
            const ASTNode_Strlit *astnode = (void *)ast;
            Vnl_StringObject *str = vnl_object_create(sizeof(*str), VNL_OBJTYPE_STRING);
            vnl_strbuf_append_s(&str->value, vnl_string_from_b(&astnode->value));
            Instruction instr = { VM_PUT, .arg = (Vnl_Object *)str };
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_BINOP: {
            const ASTNode_BinOp *astnode = (void *)ast;
            Instruction instr;
            if (astnode->op == BINOP_SET) {
                if (astnode->lhs->_ast_type != ASTTYPE_IDENT) {
                    printf(VNL_ANSICOL_RED "Error: Not assignable!" VNL_ANSICOL_RESET);
                    exit(1);
                }
                const ASTNode_Ident *ident = (void *)astnode->lhs;
                Vnl_String varname = vnl_string_from_b(&ident->value);
                exec_compile_ast(exec, astnode->rhs, compile_result);
                instr = (Instruction){ VM_DUP };
                code_append(compile_result, instr);
                instr = (Instruction){ VM_STORE, .varname = varname };
            } else {
                exec_compile_ast(exec, astnode->lhs, compile_result);
                exec_compile_ast(exec, astnode->rhs, compile_result);
                instr = (Instruction){ VM_ROT };
                code_append(compile_result, instr);
                instr = (Instruction){ (VMOpcode)astnode->op };
            }
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_CALL: {
            printf(VNL_ANSICOL_RED "TODO" VNL_ANSICOL_RESET);
            exit(1);
        } break;

        case ASTTYPE_ARRAY_LITERAL: {
            const ASTNode_ArrayLiteral *astnode = (void *)ast;
            for (size_t i = 0; i < astnode->len; ++i) {
                ASTNode *item = astnode->items[i];
                exec_compile_ast(exec, item, compile_result);
            }
            Instruction instr = { VM_MAKEARR, .makearr_len = astnode->len };
            code_append(compile_result, instr);
        } break;
    } // switch (ast->_ast_type)
}



void exec_stack_print(const Vnl_Executor *exec) {
    printf("Stack{ ");
    for (size_t i = 0; i < exec->stack.len; ++i) {
        object_print(exec->stack.stack[i]);
        if (i < exec->stack.len - 1)
            printf(", ");
    }
    printf(" }\n");
}


void exec_print_var(Vnl_String varname, const Vnl_Object *value) {
	vnl_string_print_escaped(varname);
	printf(" -> ");
	object_print(value);
	printf("\n");
}

void exec_print_vars(const Vnl_Executor *exec) {
    printf("Vars{\n");
    vnl_strmap_foreach(exec->varlist, exec_print_var);
    printf(" }\n");
}


void exec_stack_free(Vnl_Executor *exec) {
    for (size_t i = 0; i < exec->stack.len; ++i) {
        vnl_object_release(exec->stack.stack[i]);
        exec->stack.stack[i] = nullptr;
    }
    exec->stack.len = 0;
}


Vnl_String cstr2str(Vnl_CString s) {
    return (Vnl_String){ s, strlen(s) };
}

double obj_to_number(Vnl_Object *obj) {
    if (!obj && !obj_is_number(obj)) {
        return nan(0);
    }
    Vnl_NumberObject *num = (void *)obj;
    return num->value;
}

double obj_to_number_or(Vnl_Object *obj, double d) {
    if (!obj && !obj_is_number(obj)) {
        return d;
    }
    Vnl_NumberObject *num = (void *)obj;
    return num->value;
}


Vnl_Object *exec_getvar_cstr(Vnl_Executor *exec, Vnl_CString varname) {
    return vnl_exec_getvar(exec, cstr2str(varname));
}




Vnl_Executor *vnl_exec_new() {
	Vnl_Executor *exec = vnl_malloc(sizeof(*exec));
	exec->varlist = vnl_strmap_new();
	exec->error = nullptr;
	exec->stack = (Vnl_Stack){};
	return exec;
}

void vnl_exec_free(Vnl_Executor *self) {
	exec_stack_free(self);
	vnl_object_release(self->error);
	vnl_strmap_free(self->varlist);
	vnl_free(self);
}

void vnl_exec_setvar(Vnl_Executor *self, Vnl_String name, Vnl_Object *obj) {
	vnl_strmap_insert(self->varlist, name, obj);
}

Vnl_Object *vnl_exec_getvar(Vnl_Executor *self, Vnl_String name) {
	return vnl_strmap_find(self->varlist, name);
}

void vnl_exec_delvar(Vnl_Executor *self, Vnl_String name) {
	vnl_object_release(vnl_strmap_pop(self->varlist, name));
}

bool vnl_exec_string(Vnl_Executor *exec, Vnl_String source) {
 	Tokens tokens = {};

    bool debug = obj_to_number_or(exec_getvar_cstr(exec, "__debug__"), 1);
    bool debug_print_tokens = obj_to_number_or(exec_getvar_cstr(exec, "__debug_tokens__"), (double)debug);
    bool debug_print_ast = obj_to_number_or(exec_getvar_cstr(exec, "__debug_ast__"), (double)debug);
    bool debug_print_code = obj_to_number_or(exec_getvar_cstr(exec, "__debug_code__"), (double)debug);
    bool debug_print_stack = obj_to_number_or(exec_getvar_cstr(exec, "__debug_stack__"), (double)debug);
    bool debug_print_vars = obj_to_number_or(exec_getvar_cstr(exec, "__debug_vars__"), (double)debug);

    ParseError err = tokenize(&source, &tokens);
    if (err) {
        print_parseerr(err, source, nullptr, nullptr);
        tokens_free(&tokens);
        return true;
    }




    if (debug_print_tokens) {
        for (size_t i = 0; i < tokens.len; ++i) {
            const Vnl_Token *tok = &tokens.items[i];
            token_display(tok);
            printf("\n");
        }
    }


    TokenIterator titer = { &tokens, 0 };
    ASTNode *ast = nullptr;
    err = parse_statement(&titer, &ast);
    if (err) {
        print_parseerr(err, source, ast, &titer);
    } else {
        if (debug_print_ast) ast_println(ast);

        Code code = {0};
        exec_compile_ast(exec, ast, &code);

        if (debug_print_code) code_print(&code);

        if (exec_code(exec, &code)) {
            printf(VNL_ANSICOL_RED"<Error>\n"VNL_ANSICOL_RESET);
        }

        if (debug_print_stack) exec_stack_print(exec);
        if (debug_print_vars) exec_print_vars(exec);

        if (exec->stack.len) {
            object_print(exec->stack.stack[exec->stack.len-1]);
            printf("\n");
        }

        exec_stack_free(exec);
        ast_free(ast);
    }

    tokens_free(&tokens);
    return false;
}
