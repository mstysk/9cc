#include "9cc.h"

char *user_input;
Token *token;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s^ ", pos, "");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_INDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

long expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数値ではありません");
    long val = token->val;
    token = token->next;
    return val;
}

char *expect_ident() {
    if (token->kind != TK_INDENT)
        error_at(token->str, "識別子ではありません");
    char *s = strndup(token->str, token->len);
    token = token->next;
    return s;
}


void expect(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}


static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

static char *starts_with_reserved(char *p) {
    static char *kw[] = {"return", "if", "else", "while", "for"};
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        int len = strlen(kw[i]);
        if (startswith(p, kw[i]) && !isalnum(p[len])) {
            return kw[i];
        }
    }

    return NULL;
}

Token *tokenize(char *p) {
    user_input = p;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (isdigit(*p)) {
            char *q = p;
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>;={},", *p)) {
            cur = new_token(TK_RESERVED, cur, p, 1);
            cur->val = *p;
            p++;
            continue;
        }

        char *kw = starts_with_reserved(p);
        if (kw) {
            int len = strlen(kw);
            cur = new_token(TK_RESERVED, cur, p, len);
            p += len;
            continue;

        }

        if (isalnum(*p)) {
            char *q = p++;
            while(isalnum(*p))
                p++;
            cur = new_token(TK_INDENT, cur, q, p - q);
            continue;
        }

        error_at(p, "予期しない文字列です");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

