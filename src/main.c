
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#define RESET   "\e[0m"
#define BLACK   "\e[30m"
#define RED     "\e[31m"
#define GREEN   "\e[32m"
#define YELLOW  "\e[33m"
#define BLUE    "\e[34m"
#define MAGENTA "\e[35m"
#define CYAN    "\e[36m"
#define WHITE   "\e[37m"



typedef const char *cstr;
typedef struct {
    char *items;
    size_t len;
    size_t cap;
} StringBuf;

typedef struct {
    const char *items;
    size_t len;
} String;


size_t next_pow2(size_t x) {
  uint64_t p = 1;
  while (p < x) p *= 2;
  return p;
}

size_t round8(size_t x) {
    const int round_to = 8;
    return (x + (round_to - 1)) & ~(round_to - 1);
}


void strbuf_reserve(StringBuf *sb, size_t size) {
    if (size <= sb->cap - sb->len) {
        return;
    }

    size_t newcap = round8(sb->len + size);

    sb->items = realloc(sb->items, newcap);
    sb->cap = newcap;
}


void strbuf_appends(StringBuf *sb, String s) {
    strbuf_reserve(sb, s.len);
    memcpy(sb->items + sb->len, s.items, s.len);
    sb->len += s.len;
}

void strbuf_free(StringBuf *sb) {
    free(sb->items);
}

String strbuf_string(const StringBuf *sb) {
    return (String){
        .items = sb->items,
        .len = sb->len,
    };
}

#define auto_strbuf_t StringBuf _CLEANUP(strbuf_free)

bool string_eq(String s, cstr cs) {
    return strncmp(s.items, cs, s.len) == 0;
}

bool string_seq(String s1, String s2) {
    return (s1.len == s2.len) && (strncmp(s1.items, s2.items, s1.len) == 0);
}

bool string_hasprefix(String, cstr);
bool string_hassuffix(String, cstr);
bool string_shasprefix(String, String);
bool string_shassuffix(String, String);

String string_remprefix(String, cstr);
String string_remsuffix(String, cstr);
String string_sremprefix(String, String);
String string_sremsuffix(String, String);



// Check if string has prefix (C string)
bool string_hasprefix(String s, cstr prefix) {
    size_t prefix_len = strlen(prefix);
    if (s.len < prefix_len) {
        return false;
    }
    return strncmp(s.items, prefix, prefix_len) == 0;
}

// Check if string has suffix (C string)
bool string_hassuffix(String s, cstr suffix) {
    size_t suffix_len = strlen(suffix);
    if (s.len < suffix_len) {
        return false;
    }
    return strncmp(s.items + s.len - suffix_len, suffix, suffix_len) == 0;
}

// Check if string has prefix (String)
bool string_shasprefix(String s, String prefix) {
    if (s.len < prefix.len) {
        return false;
    }
    return strncmp(s.items, prefix.items, prefix.len) == 0;
}

// Check if string has suffix (String)
bool string_shassuffix(String s, String suffix) {
    if (s.len < suffix.len) {
        return false;
    }
    return strncmp(s.items + s.len - suffix.len, suffix.items, suffix.len) == 0;
}

// Remove prefix (C string) - returns new string view
String string_remprefix(String s, cstr prefix) {
    size_t prefix_len = strlen(prefix);
    if (s.len >= prefix_len && strncmp(s.items, prefix, prefix_len) == 0) {
        return (String){s.items + prefix_len, s.len - prefix_len};
    }
    return s; // Return original if prefix not found
}

// Remove suffix (C string) - returns new string view
String string_remsuffix(String s, cstr suffix) {
    size_t suffix_len = strlen(suffix);
    if (s.len >= suffix_len &&
        strncmp(s.items + s.len - suffix_len, suffix, suffix_len) == 0) {
        return (String){s.items, s.len - suffix_len};
    }
    return s; // Return original if suffix not found
}

// Remove prefix (String) - returns new string view
String string_sremprefix(String s, String prefix) {
    if (s.len >= prefix.len && strncmp(s.items, prefix.items, prefix.len) == 0) {
        return (String){s.items + prefix.len, s.len - prefix.len};
    }
    return s; // Return original if prefix not found
}

// Remove suffix (String) - returns new string view
String string_sremsuffix(String s, String suffix) {
    if (s.len >= suffix.len &&
        strncmp(s.items + s.len - suffix.len, suffix.items, suffix.len) == 0) {
        return (String){s.items, s.len - suffix.len};
    }
    return s; // Return original if suffix not found
}

String string_shl(String s, size_t sh) {
    sh = sh > s.len ? s.len : sh;
    s.items += sh;
    s.len -= sh;
    return s;
}

String string_shr(String s, size_t sh) {
    sh = sh > s.len ? s.len : sh;
    s.len -= sh;
    return s;
}


String string_ltrim(String s) {
    while (s.len && strchr(" \t\n\v\f\r", *s.items)) {
        s = string_shl(s, 1);
    }
    return s;
}

String string_rtrim(String s) {
    while (s.len && strchr(" \t\n\v\f\r", s.items[s.len-1])) {
        s = string_shr(s, 1);
    }
    return s;
}

String string_trim(String s) {
    return string_ltrim(string_rtrim(s));
}

void string_display(String s, FILE *fp) {
    fprintf(fp, "%.*s", (int)s.len, s.items);
}

void string_print(String s) {
    string_display(s, stdout);
}

void string_println(String s) {
    string_display(s, stdout);
    printf("\n");
}


void string_print_escaped(String s) {
    putchar('"');
    for (size_t i = 0; i < s.len; ++i) {
        char c = s.items[i];
        switch (c) {
            case '\a':
                printf("\\a");
            break;
            case '\n':
                printf("\\n");
            break;
            case '\t':
                printf("\\t");
            break;
            case '\v':
                printf("\\v");
            break;
            case '\b':
                printf("\\b");
            break;
            case '\r':
                printf("\\r");
            break;
            case '\f':
                printf("\\f");
            break;
            case '\"':
                printf("\\\"");
            break;
            case '\'':  // not used since string uses " for borders
                printf("'");
            break;
            case '\\':
                printf("\\\\");
            break;
            default:
                putchar(c);
            break;
        }
    }
    putchar('"');
}




typedef struct {
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
        String value;
        double numval;
    };
} Token;

static const cstr TOKTYPE_NAMES[] = {
    [TOK_IDENT]  = "Ident",
    [TOK_NUMLIT] = "Numlit",
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
    Token *items;
    size_t len;
    size_t cap;
} Tokens;


void tokens_push(Tokens *tokens, Token token) {
    if (tokens->cap == tokens->len) {
        size_t newcap = tokens->cap;
        newcap = newcap ? newcap * 2 : 32;
        tokens->items = realloc(tokens->items, newcap * sizeof(Token));
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

void token_display(const Token *tok) {
    printf("Token{");
    printf("type=%s", TOKTYPE_NAMES[tok->type]);
    switch (tok->type) {
        case TOK_NUMLIT:
            printf(", value=%g", tok->numval);
        break;

        case TOK_STRLIT:
            printf(", value=");
            string_print_escaped(tok->value);
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
            string_print(tok->value);
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


ParseError tokenize(String *source, Tokens *tokens) {
    ParseError err = PARSEERR_OK;
    *source = string_ltrim(*source);

    while (source->len) {
        switch (source->items[0]) {
            case '(': {
                *source = string_shl(*source, 1);
                Token tok = { TOK_LPAREN, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case ')': {
                *source = string_shl(*source, 1);
                Token tok = { TOK_RPAREN, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case '[': {
                *source = string_shl(*source, 1);
                Token tok = { TOK_LBRACK, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case ']': {
                *source = string_shl(*source, 1);
                Token tok = { TOK_RBRACK, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case ',': {
                *source = string_shl(*source, 1);
                Token tok = { TOK_COMMA, .value = {0} };
                tokens_push(tokens, tok);
            } break;

            case '"': {
                *source = string_shl(*source, 1);
                Token tok = { TOK_STRLIT, .value = {source->items, 0} };
                while(source->len && *source->items != '"') {
                    *source = string_shl(*source, 1);
                    tok.value.len++;
                }
                if (!source->len) {
                    err = PARSEERR_AST_UNFINISHED_STRLIT;
                    goto return_err;
                }
                *source = string_shl(*source, 1);
                tokens_push(tokens, tok);
            } break;

            case '0' ... '9': {
                char *start, *end;
                start = end = (char *)source->items;
                double numval = strtod(start, &end);
                if (start == end) {
                    err = PARSEERR_TOK_NUMBER;
                    goto return_err;
                }
                *source = string_shl(*source, end - start);
                Token tok = { TOK_NUMLIT, .numval = numval };
                tokens_push(tokens, tok);
            } break;

            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_': {
                Token tok = { TOK_IDENT, .value = *source };
                tok.value.len = 0;
                while (source->len && (isalnum(*source->items) || *source->items == '_')) {
                    tok.value.len ++;
                    *source = string_shl(*source, 1);
                }
                tokens_push(tokens, tok);
            } break;

            case '+': case '-':
            case '*': case '/':
            case '%': case '=': {
                Token tok = { TOK_OP, .value = { source->items, 1 } };
                *source = string_shl(*source, 1);
                tokens_push(tokens, tok);
            } break;

            default:
                err = PARSEERR_TOK_UNKNOWN;
                goto return_err;
            break;

        } // switch (source.items[0])

        *source = string_ltrim(*source);
    } // while (source->len)


    return_ok:
        return err;

    return_err:
        tokens_free(tokens);
        return err;
}





void console_clear() {
    printf("\033[H\033[2J");
}


void console_read(StringBuf *buffer) {
    buffer->len = getline(&buffer->items, &buffer->cap, stdin);
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
    cstr str;
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
    StringBuf value;
} ASTNode_Ident;

typedef struct {
    _ASTNODEBASE();
    StringBuf value;
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

Token titer_get(TokenIterator *titer) {
    if (titer->off < titer->tokens->len) {
        return titer->tokens->items[titer->off++];
    }
    return (Token){ TOK_EOF };
}

Token titer_peek(const TokenIterator *titer) {
    if (titer->off < titer->tokens->len) {
        return titer->tokens->items[titer->off];
    }
    return (Token){ TOK_EOF };
}


void ast_free(ASTNode *node) {
    switch (node->_ast_type) {
            case ASTTYPE_NUMLIT: {
                free(node);
            } break;

            case ASTTYPE_STRLIT: {
                ASTNode_Strlit *strlit = (void *)node;
                strbuf_free(&strlit->value);
                free(node);
            } break;

            case ASTTYPE_IDENT: {
                ASTNode_Ident *ident = (ASTNode_Ident *)node;
                strbuf_free(&ident->value);
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


OpInfo get_op_info(String strop) {
    const size_t numops = sizeof(OPINFO_TABLE)/sizeof(OPINFO_TABLE[0]);
    for (size_t i = 0; i < numops; ++i) {
        OpInfo opinfo = OPINFO_TABLE[i];
        if (string_eq(strop, opinfo.str)) {
            return opinfo;
        }
    }
    printf(RED "Invalid strop: %.*s\n" RESET, (int)strop.len, strop.items);
    abort();
}


ParseError parse_expression(TokenIterator *titer, ASTNode **expr, float min_bp);

ParseError parse_array_literal(TokenIterator *titer, ASTNode **node) {
    Token tok;
    ParseError err = PARSEERR_OK;
    *node = nullptr;

    tok = titer_peek(titer);
    if (tok.type != TOK_LBRACK) {
        err = PARSEERR_AST_EXPECTED_LBRACK;
        goto return_failure;
    }
    titer_get(titer);

    ASTNode_ArrayLiteral *arr = malloc(sizeof(*arr));
    *arr = (ASTNode_ArrayLiteral){ ASTTYPE_ARRAY_LITERAL, RVALUE };

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
    Token tok;
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
            ASTNode_Ident *ident = malloc(sizeof(*ident));
            *ident = (ASTNode_Ident) { ASTTYPE_IDENT, LVALUE, (StringBuf){} };
            strbuf_appends(&ident->value, tok.value);
            lhs = (void *)ident;
        } break;

        case TOK_STRLIT: {
            titer_get(titer);
            ASTNode_Strlit *strlit = malloc(sizeof(*strlit));
            *strlit = (ASTNode_Strlit){ ASTTYPE_STRLIT, LVALUE, (StringBuf){} };
            strbuf_appends(&strlit->value, tok.value);
            lhs = (void *)strlit;
        } break;

        case TOK_NUMLIT: {
            titer_get(titer);
            ASTNode_Numlit *numlit = malloc(sizeof(*numlit));
            *numlit = (ASTNode_Numlit){ ASTTYPE_NUMLIT, RVALUE, tok.numval };
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

        ASTNode_BinOp *binop = malloc(sizeof(*binop));
        *binop = (ASTNode_BinOp){ ASTTYPE_BINOP, RVALUE, opinfo.kind, lhs, rhs };
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
    Token tok = titer_peek(titer);
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

    cstr valtype = ast->_ast_valtype == LVALUE ? "LValue" : "RValue";

    switch (ast->_ast_type) {
        case ASTTYPE_NUMLIT: {
            ASTNode_Numlit *numlit = (void *)ast;
            printf("Numlit(valtype=%s, value=%g)", valtype, numlit->value);
        } break;

        case ASTTYPE_IDENT: {
            ASTNode_Ident *ident = (void *)ast;
            printf("Ident(valtype=%s, value=%.*s)", valtype, (int)ident->value.len, ident->value.items);
        } break;

        case ASTTYPE_STRLIT: {
            ASTNode_Strlit *strlit = (void *)ast;
            printf("Strlit(valtype=%s, value=%.*s)", valtype, (int)strlit->value.len, strlit->value.items);
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


void print_parseerr(ParseError err, String line, const ASTNode *ast, const TokenIterator *titer) {
    switch (err) {
        case PARSEERR_OK: {
            printf(GREEN "Error: Success!\n" RESET);
        } break;

        case PARSEERR_TOK_UNKNOWN: {
            printf(RED "Error: Unknown character '%c' (%d)\n" RESET, *line.items, *line.items);
        } break;

        case PARSEERR_TOK_NUMBER: {
            printf(RED "Error: Cannot parse number from position: \"\n");
            if (line.len <= 10) {
                string_print(line);
                printf("\"\n"RESET);
            } else {
                printf("%.*s" CYAN "..." RED "\"\n" RESET, 5, line.items);
            }
        } break;

        case PARSEERR_AST_UNFINISHED_STRLIT: {
            printf(RED "Error: Unfinished string literal" RESET);
        } break;

        case PARSEERR_AST_EXPECTED_IDENT: {
            printf(RED "Error: Expected ident, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_NUMLIT: {
            printf(RED "Error: Expected numlit, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_LPAREN: {
            printf(RED "Error: Expected lparen, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_RPAREN: {
            printf(RED "Error: Expected rparen, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_LBRACK: {
            printf(RED "Error: Expected lbrack, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_RBRACK: {
            printf(RED "Error: Expected rbrack, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_COMMA: {
            printf(RED "Error: Expected comma, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_BINOP: {
            printf(RED "Error: Expected binop, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_EOF: {
            printf(RED "Error: Expected eof, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_LVALUE: {
            printf(RED "Error: Expected lvalue, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

        case PARSEERR_AST_EXPECTED_RVALUE: {
            printf(RED "Error: Expected rvalue, got ");
            Token tok = titer_peek(titer);
            token_display(&tok);
            printf("\n");
        } break;

    }
}



typedef struct Object Object;

typedef enum {
    OBJTYPE_NUMBER,
    OBJTYPE_STRING,
    OBJTYPE_ARRAY,
} ObjectType;

#define EMPTY_OBJBASE_INITIALIZER(objtype) { 0, (objtype) }

#define _OBJECTBASE() struct { size_t refcount; ObjectType type; } __obj_base__

struct Object {
    _OBJECTBASE();
};

typedef struct {
    _OBJECTBASE();
    double value;
} Object_Number;

typedef struct {
    _OBJECTBASE();
    StringBuf value;
} Object_String;

typedef struct {
    _OBJECTBASE();
    Object **items;
    size_t len;
    size_t cap;
} Object_Array;

typedef struct {
    Object *begin, *end;
    size_t len;
} GCList;

typedef struct {
    cstr name;
    Object *value;
} Variable;

typedef struct {
    Variable *items;
    size_t len;
    size_t cap;
} VarList;



typedef struct {
    Object **stack;
    size_t stack_len;
    size_t stack_cap;
    VarList varlist;
    Object *error;
} Executor;

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
        Object *arg;
        String varname;
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
        code->items = realloc(code->items, newcap * sizeof(Token));
        code->cap = newcap;
    }
    code->items[code->len++] = instr;
}



void object_print(Object *obj) {
    switch (obj->__obj_base__.type) {
        case OBJTYPE_NUMBER:
            printf("%g", ((Object_Number*)obj)->value);
        break;

        case OBJTYPE_STRING: {
            Object_String *strobj = (void *)obj;
            string_print_escaped(strbuf_string(&strobj->value));
        } break;

        case OBJTYPE_ARRAY:
            printf("[");
            Object_Array *arr = (void *)obj;
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
                string_println(instr.varname);
            } break;

            case VM_STORE: {
                printf("STORE ");
                string_println(instr.varname);
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



Object_Number *exec_create_number(Executor *exec, double value) {
    Object_Number *obj = malloc(sizeof(*obj));
    *obj = (Object_Number){ EMPTY_OBJBASE_INITIALIZER(OBJTYPE_NUMBER), value };
    return obj;
}

void incref(Object *obj) {
    obj->__obj_base__.refcount++;
}

void decref(Object *obj) {
    obj->__obj_base__.refcount--;
}

void exec_decdrop(Executor *exec, Object *obj);

void exec_stack_push(Executor *exec, Object *obj) {
    incref(obj);
    if (exec->stack_cap == exec->stack_len) {
        size_t newcap = exec->stack_cap;
        newcap = newcap ? newcap * 2 : 32;
        exec->stack = realloc(exec->stack, newcap * sizeof(Token));
        exec->stack_cap = newcap;
    }
    exec->stack[exec->stack_len++] = obj;
}


Object *exec_stack_pop(Executor *exec) {
    if (exec->stack_len == 0) {
        return nullptr;
    } else {
        Object *obj = exec->stack[--exec->stack_len];
        return obj;
    }
}


void exec_set_error(Executor *exec, Object *err_obj) {
    exec_decdrop(exec, exec->error);
    exec->error = err_obj;
    incref(err_obj);
}



bool obj_is_number(Object *obj) {
    return obj && obj->__obj_base__.type == OBJTYPE_NUMBER;
}


bool isintegral(double x) {
    double i;
    return modf(x, &i) == 0.0;
}

bool obj_is_integer(Object *obj) {
    return obj && obj->__obj_base__.type == OBJTYPE_NUMBER && isintegral(((Object_Number *)obj)->value);
}

bool obj_is_string(Object *obj) {
    return obj && obj->__obj_base__.type == OBJTYPE_STRING;
}

cstr objtype_as_str(Object *obj) {
    static const cstr OBJTYPE2STR[] = {
        [OBJTYPE_NUMBER] = "number",
        [OBJTYPE_ARRAY] = "array",
    };
    return OBJTYPE2STR[obj->__obj_base__.type];
}

int64_t obj_as_integer(Object *obj) {
    return ((Object_Number *)obj)->value;
}

uint64_t obj_as_uinteger(Object *obj) {
    return ((Object_Number *)obj)->value;
}

void error_invalid_binop_args(OpKind op, Object *a, Object *b) {
    if (!a && !b) {
        printf(RED "Error: not enough arguments, stack exceeded!\n" RESET);
        exit(1);
    } else {
        OpInfo opinfo = OPINFO_TABLE[op];
        printf(
            RED "Error: "
            "Unsupported arguments for operator '%s':"
            " <%s> and <%s>\n" RESET,
            opinfo.str,
            objtype_as_str(a),
            objtype_as_str(b)
        );
    }
}






void exec_decdrop(Executor *exec, Object *obj) {
    decref(obj);

    if (obj->__obj_base__.refcount == 0) {
        // printf("deleting: ");
        // object_print(obj);
        // printf("\n");
        switch (obj->__obj_base__.type) {
            case OBJTYPE_NUMBER: {
                Object_Number *num = (void *)obj;
                free(num);
            } break;

            case OBJTYPE_STRING: {
                Object_String *str = (void *)obj;
                strbuf_free(&str->value);
                free(str);
            } break;

            case OBJTYPE_ARRAY: {
                Object_Array *arr = (void *)obj;
                for (size_t i = 0; i < arr->len; ++i) {
                    exec_decdrop(exec, arr->items[i]);
                }
                free(arr);
            } break;
        }
    }
}





Object *exec_get_var(Executor *exec, String name) {
    for (size_t i = 0; i < exec->varlist.len; ++i) {
        Variable *var = &exec->varlist.items[i];
        if (string_eq(name, var->name)) {
            return var->value;
        }
    }
    return nullptr;
}


void exec_set_var(Executor *exec, String name, Object *obj) {
    for (size_t i = 0; i < exec->varlist.len; ++i) {
        Variable *var = &exec->varlist.items[i];
        if (string_eq(name, var->name)) {
            exec_decdrop(exec, var->value);
            var->value = obj;
            return;
        }
    }

    if (exec->varlist.cap == exec->varlist.len) {
        size_t newcap = exec->varlist.cap;
        newcap = newcap ? newcap * 2 : 32;
        exec->varlist.items = realloc(exec->varlist.items, newcap * sizeof(Token));
        exec->varlist.cap = newcap;
    }
    exec->varlist.items[exec->varlist.len++] = (Variable){
        .name = strndup(name.items, name.len),
        .value = obj
    };
    incref(obj);
}



void exec_clear_vars(Executor *exec) {
    for (size_t i = 0; i < exec->varlist.len; ++i) {
        Variable *var = &exec->varlist.items[i];
        free((void *)var->name);
        exec_decdrop(exec, var->value);
    }
    free(exec->varlist.items);
    exec->varlist = (VarList){};
}




void obj_array_push(Object_Array *arr, Object *obj) {
    if (arr->cap == arr->len) {
        size_t newcap = arr->cap;
        newcap = newcap ? newcap * 2 : 32;
        arr->items = realloc(arr->items, newcap * sizeof(Token));
        arr->cap = newcap;
    }
    arr->items[arr->len++] = obj;
}


void obj_arr_reverse(Object_Array *arr) {
    for (size_t i = 0; i < arr->len / 2; ++i) {
        Object *tmp = arr->items[i];
        arr->items[i] = arr->items[arr->len - 1 - i];
        arr->items[arr->len - 1 - i] = tmp;
    }
}






ExecError exec_code(Executor *exec, const Code *code) {
    for (size_t pc = 0; pc < code->len; ++pc) {
        Instruction instr = code->items[pc];
        switch (instr.opcode) {
            case VM_SET: {
                printf(RED "TODO: Illegal instruction\n" RESET);
                exit(1);
            } break;

            case VM_ADD: {
                Object *a = exec_stack_pop(exec);
                Object *b = exec_stack_pop(exec);

                if (obj_is_number(a) && obj_is_number(b)) {
                    Object_Number *anum = (void *)a;
                    Object_Number *bnum = (void *)b;
                    Object_Number *result = exec_create_number(exec, anum->value + bnum->value);
                    exec_decdrop(exec, a);
                    exec_decdrop(exec, b);
                    exec_stack_push(exec, (Object *)result);
                } else if (obj_is_string(a) && obj_is_string(b)) {
                    Object_String *astr = (void *)a;
                    Object_String *bstr = (void *)b;
                    Object_String *result = malloc(sizeof(*result));
                    *result = (Object_String){ EMPTY_OBJBASE_INITIALIZER(OBJTYPE_STRING) };
                    strbuf_appends(&result->value, strbuf_string(&astr->value));
                    strbuf_appends(&result->value, strbuf_string(&bstr->value));
                    exec_decdrop(exec, a);
                    exec_decdrop(exec, b);
                    exec_stack_push(exec, (Object *)result);
                } else {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }
            } break;

            case VM_SUB: {
                Object *a = exec_stack_pop(exec);
                Object *b = exec_stack_pop(exec);

                if (!obj_is_number(a) || !obj_is_number(b)) {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }

                Object_Number *anum = (void *)a;
                Object_Number *bnum = (void *)b;
                Object_Number *result = exec_create_number(exec, anum->value - bnum->value);
                exec_decdrop(exec, a);
                exec_decdrop(exec, b);
                exec_stack_push(exec, (Object *)result);
            } break;

            case VM_MUL: {
                Object *a = exec_stack_pop(exec);
                Object *b = exec_stack_pop(exec);

                if (obj_is_number(a) && obj_is_number(b)) {
                    Object_Number *anum = (void *)a;
                    Object_Number *bnum = (void *)b;
                    Object_Number *result = exec_create_number(exec, anum->value * bnum->value);
                    exec_decdrop(exec, a);
                    exec_decdrop(exec, b);
                    exec_stack_push(exec, (Object *)result);
                } else if (obj_is_integer(a) && obj_is_string(b) && obj_as_integer(a) >= 0) {
                    size_t times = obj_as_integer(a);
                    Object_String *result = malloc(sizeof(*result));
                    *result = (Object_String){ EMPTY_OBJBASE_INITIALIZER(OBJTYPE_STRING) };
                    Object_String *bstr = (void *)b;
                    for (size_t i = 0; i < times; ++i) {
                        strbuf_appends(&result->value, strbuf_string(&bstr->value));
                    }
                    exec_decdrop(exec, a);
                    exec_decdrop(exec, b);
                    exec_stack_push(exec, (Object *)result);
                } else if (obj_is_integer(b) && obj_is_string(a) && obj_as_integer(b) >= 0) {
                    size_t times = obj_as_integer(b);
                    Object_String *result = malloc(sizeof(*result));
                    *result = (Object_String){ EMPTY_OBJBASE_INITIALIZER(OBJTYPE_STRING) };
                    Object_String *astr = (void *)a;
                    for (size_t i = 0; i < times; ++i) {
                        strbuf_appends(&result->value, strbuf_string(&astr->value));
                    }
                    exec_decdrop(exec, a);
                    exec_decdrop(exec, b);
                    exec_stack_push(exec, (Object *)result);
                } else {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }
            } break;

            case VM_DIV: {
                Object *a = exec_stack_pop(exec);
                Object *b = exec_stack_pop(exec);

                if (!obj_is_number(a) || !obj_is_number(b)) {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }

                Object_Number *anum = (void *)a;
                Object_Number *bnum = (void *)b;
                Object_Number *result = exec_create_number(exec, anum->value / bnum->value);
                exec_decdrop(exec, a);
                exec_decdrop(exec, b);
                exec_stack_push(exec, (Object *)result);
            } break;

            case VM_MOD: {
                Object *a = exec_stack_pop(exec);
                Object *b = exec_stack_pop(exec);

                if (!obj_is_number(a) || !obj_is_number(b)) {
                    error_invalid_binop_args(BINOP_ADD, a, b);
                    return EXEC_ERR;
                }

                Object_Number *anum = (void *)a;
                Object_Number *bnum = (void *)b;

                Object_Number *result = exec_create_number(exec, fmod(anum->value, bnum->value));
                exec_decdrop(exec, a);
                exec_decdrop(exec, b);
                exec_stack_push(exec, (Object *)result);
            } break;

            case VM_LOAD: {
                String varname = instr.varname;
                Object *obj = exec_get_var(exec, varname);
                if (!obj) {
                    printf(RED "Error: Unknown variable: ");
                    string_println(varname);
                    printf(RESET);
                    return EXEC_ERR;
                }
                exec_stack_push(exec, obj);
            } break;

            case VM_STORE: {
                String varname = instr.varname;
                Object *obj = exec_stack_pop(exec);
                if (!obj) {
                    printf(RED "Error: No value to store - stack is empty!\n" RESET);
                    return EXEC_ERR;
                }
                exec_set_var(exec, varname, obj);
            } break;

            case VM_ROT: {
               if (exec->stack_len < 2) {
                   printf(RED "Error: not enough values to ROT!");
                   return EXEC_ERR;
               }
               Object *tmp = exec->stack[exec->stack_len - 1];
               exec->stack[exec->stack_len - 1] = exec->stack[exec->stack_len - 2];
               exec->stack[exec->stack_len - 2] = tmp;
            } break;

            case VM_DUP: {
                if (!exec->stack_len) {
                    printf(RED "Error: Cannot DUP - stack is empty!\n" RESET);
                    return EXEC_ERR;
                } else {
                    exec_stack_push(exec, exec->stack[exec->stack_len-1]);
                }
            } break;

            case VM_PUT: {
                exec_stack_push(exec, instr.arg);
            } break;

            case VM_MAKEARR: {
                size_t arrsize = instr.makearr_len;
                Object_Array *arr = malloc(sizeof(*arr));
                *arr = (Object_Array){ EMPTY_OBJBASE_INITIALIZER(OBJTYPE_ARRAY) };
                for (size_t i = 0; i < arrsize; ++i) {
                    Object *obj = exec_stack_pop(exec);
                    obj_array_push(arr, obj);
                }
                obj_arr_reverse(arr);
                exec_stack_push(exec, (Object *)arr);
            } break;

        }
    }
    return EXEC_OK;
}


void exec_compile_ast(Executor *exec, const ASTNode *ast, Code *compile_result) {
    switch (ast->_ast_type) {
        case ASTTYPE_NUMLIT: {
            const ASTNode_Numlit *astnode = (void *)ast;
            Object_Number *obj = exec_create_number(exec, astnode->value);
            Instruction instr = { VM_PUT, .arg = (Object *)obj };
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_IDENT: {
            const ASTNode_Ident *astnode = (void *)ast;
            String varname = strbuf_string(&astnode->value);
            Instruction instr = { VM_LOAD, .varname = varname };
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_STRLIT: {
            const ASTNode_Strlit *astnode = (void *)ast;
            Object_String *str = malloc(sizeof(*str));
            *str = (Object_String){ EMPTY_OBJBASE_INITIALIZER(OBJTYPE_STRING) };
            strbuf_appends(&str->value, strbuf_string(&astnode->value));
            Instruction instr = { VM_PUT, .arg = (Object *)str };
            code_append(compile_result, instr);
        } break;

        case ASTTYPE_BINOP: {
            const ASTNode_BinOp *astnode = (void *)ast;
            Instruction instr;
            if (astnode->op == BINOP_SET) {
                if (astnode->lhs->_ast_type != ASTTYPE_IDENT) {
                    printf(RED "Error: Not assignable!" RESET);
                    exit(1);
                }
                const ASTNode_Ident *ident = (void *)astnode->lhs;
                String varname = strbuf_string(&ident->value);
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
            printf(RED "TODO" RESET);
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



void exec_stack_print(const Executor *exec) {
    printf("Stack{ ");
    for (size_t i = 0; i < exec->stack_len; ++i) {
        object_print(exec->stack[i]);
        if (i < exec->stack_len - 1)
            printf(", ");
    }
    printf(" }\n");
}


void exec_print_vars(const Executor *exec) {
    printf("Vars{\n");
    for (size_t i = 0; i < exec->varlist.len; ++i) {
        const Variable *var = &exec->varlist.items[i];
        printf("   %s -> ", var->name);
        object_print(var->value);
        printf("\n");
        // if (i < exec->stack_len - 1)
        //     printf(", ");
    }
    printf(" }\n");
}


void exec_stack_free(Executor *exec) {
    for (size_t i = 0; i < exec->stack_len; ++i) {
        exec_decdrop(exec, exec->stack[i]);
        exec->stack[i] = nullptr;
    }
    exec->stack_len = 0;
}


String cstr2str(cstr s) {
    return (String){ s, strlen(s) };
}

double obj_to_number(Object *obj) {
    if (!obj && !obj_is_number(obj)) {
        return nan(0);
    }
    Object_Number *num = (void *)obj;
    return num->value;
}

double obj_to_number_or(Object *obj, double d) {
    if (!obj && !obj_is_number(obj)) {
        return d;
    }
    Object_Number *num = (void *)obj;
    return num->value;
}


Object *exec_getvar_cstr(Executor *exec, cstr varname) {
    return exec_get_var(exec, cstr2str(varname));
}





void exec_string(Executor *exec, String source) {
    Tokens tokens = {};

    bool debug = obj_to_number_or(exec_getvar_cstr(exec, "__debug__"), 0);
    bool debug_print_tokens = obj_to_number_or(exec_getvar_cstr(exec, "__debug_tokens__"), (double)debug);
    bool debug_print_ast = obj_to_number_or(exec_getvar_cstr(exec, "__debug_ast__"), (double)debug);
    bool debug_print_code = obj_to_number_or(exec_getvar_cstr(exec, "__debug_code__"), (double)debug);
    bool debug_print_stack = obj_to_number_or(exec_getvar_cstr(exec, "__debug_stack__"), (double)debug);
    bool debug_print_vars = obj_to_number_or(exec_getvar_cstr(exec, "__debug_vars__"), (double)debug);
    bool debug_print_free = obj_to_number_or(exec_getvar_cstr(exec, "__debug_free__"), (double)debug);


    ParseError err = tokenize(&source, &tokens);
    if (err) {
        print_parseerr(err, source, nullptr, nullptr);
        tokens_free(&tokens);
        return;
    }




    if (debug_print_tokens) {
        for (size_t i = 0; i < tokens.len; ++i) {
            const Token *tok = &tokens.items[i];
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
            printf(RED"<Error>\n"RESET);
        }

        if (debug_print_stack) exec_stack_print(exec);
        if (debug_print_vars) exec_print_vars(exec);

        if (exec->stack_len) {
            object_print(exec->stack[exec->stack_len-1]);
            printf("\n");
        }

        exec_stack_free(exec);
        ast_free(ast);
    }

    tokens_free(&tokens);

}

int main() {
    StringBuf linebuf = {};
    Executor exec = {0};

    while (true) {
        printf("\e[34m>>\e[0m ");
        console_read(&linebuf);
        String line = strbuf_string(&linebuf);
        line = string_remsuffix(line, "\n");

        if (string_eq(line, "")) {
            continue;
        }

        if (string_eq(line, "exit")) {
            printf("\e[31mExiting...\e[0m\n");
            break;
        }

        if (string_eq(line, "clear")) {
            printf("\e[1;1H\e[2J");
            continue;
        }

        exec_string(&exec, line);

    }
    strbuf_free(&linebuf);

    return 0;
}
