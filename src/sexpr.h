#ifndef SEXPR_H
#define SEXPR_H

typedef enum { TYPE_INT, TYPE_DOUBLE, TYPE_STRING, TYPE_SYMBOL, TYPE_CONS, TYPE_NIL } sExprType;

typedef struct sExpr {
    sExprType type;
    union {
        long integer;
        double dbl;
        char *string;
        char *symbol;
        struct {
            struct sExpr *car;
            struct sExpr *cdr;
        } cons;
    } value;
} sExpr;

typedef struct {
    char **items;
    int count;
    int pos;
} TokenStream;

sExpr* create_env();
sExpr* get_symbol(sExpr* target, sExpr* symbol, sExpr* value);
sExpr* lookup(sExpr* symbol);
sExpr* set(sExpr* symbol, sExpr* value);

TokenStream tokenize(const char* input);
void free_tokens(TokenStream *ts);
sExpr* parse_sexpr(TokenStream *ts);

// Constructors 
sExpr* create_int(long value);
sExpr* create_double(double value);
sExpr* create_string(const char *value);
sExpr* create_symbol(const char *s);
sExpr* cons(sExpr *car, sExpr *cdr);
sExpr* car(sExpr *e);
sExpr* cdr(sExpr *e);

// Utilities
int isnil(sExpr *e);
int issymbol(sExpr *e);
int isnumber(sExpr *e);
int isstring(sExpr *e);
int islist(sExpr *e);
int sExpr_to_bool(sExpr *e);
void print_sExpr(sExpr *e);
void free_sExpr(sExpr *e);

// Singletons
extern sExpr *NIL;
extern sExpr *TRUE;

// Arithmetic & Comparisons
sExpr* add(sExpr *a, sExpr *b);
sExpr* sub(sExpr *a, sExpr *b);
sExpr* mul(sExpr *a, sExpr *b);
sExpr* divide(sExpr *a, sExpr *b);
sExpr* mod(sExpr *a, sExpr *b);

sExpr* lt(sExpr *a, sExpr *b);
sExpr* gt(sExpr *a, sExpr *b);
sExpr* lte(sExpr *a, sExpr *b);
sExpr* gte(sExpr *a, sExpr *b);
sExpr* eq(sExpr *a, sExpr *b);
sExpr* not_sExpr(sExpr *a);

#endif
