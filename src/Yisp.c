#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sexpr.h"

sExpr *NIL;
sExpr *TRUE;

//Constructors
sExpr* create_int(long value) {
    sExpr *e = (sExpr *)malloc(sizeof(sExpr));
    e->type = TYPE_INT;
    e->value.integer = value;
    return e;
}

sExpr* create_double(double value){
    sExpr *e = (sExpr *)malloc(sizeof(sExpr));
    e->type = TYPE_DOUBLE;
    e->value.dbl = value;
    return e;
}

sExpr* create_string(const char *value){
    sExpr *e = (sExpr *)malloc(sizeof(sExpr));
    e->type = TYPE_STRING;
    e->value.string = strdup(value);
    return e;
}

sExpr* create_symbol(const char *s){
    sExpr *e = (sExpr *)malloc(sizeof(sExpr));
    e->type = TYPE_SYMBOL;
    e->value.symbol = strdup(s);
    return e;
}

sExpr* cons(sExpr *car, sExpr *cdr){
    sExpr *e = (sExpr *)malloc(sizeof(sExpr));
    e->type = TYPE_CONS;
    e->value.cons.car = car;
    e->value.cons.cdr = cdr;
    return e;
}

sExpr* car(sExpr *e){
    return (e->type == TYPE_CONS) ? e->value.cons.car : NIL;
}

sExpr* cdr(sExpr *e){
    return (e->type == TYPE_CONS) ? e->value.cons.cdr : NIL;
}

int isnil(sExpr *e)    { return (e->type == TYPE_NIL); }

int issymbol(sExpr *e) { return (e->type == TYPE_SYMBOL); }

int isnumber(sExpr *e) { return (e->type == TYPE_INT || e->type == TYPE_DOUBLE); }

int isstring(sExpr *e) { return (e->type == TYPE_STRING); }

int islist(sExpr *e) {
    while (e->type == TYPE_CONS) e = cdr(e);
    return (e->type == TYPE_NIL);
}

int sExpr_to_bool(sExpr *e) {
    return (e->type == TYPE_NIL) ? 0 : 1;
}

void print_sExpr(sExpr *e) {
    switch (e->type) {
        case TYPE_INT:
            printf("%ld", e->value.integer);
            break;

        case TYPE_DOUBLE:
            printf("%f", e->value.dbl);
            break;

        case TYPE_STRING:
            printf("\"%s\"", e->value.string);
            break;

        case TYPE_SYMBOL:
            printf("%s", e->value.symbol);
            break;

        case TYPE_NIL:
            printf("()");
            break;

        case TYPE_CONS: {
            sExpr *head = e->value.cons.car;
            sExpr *tail = e->value.cons.cdr;

            if (head && head->type == TYPE_SYMBOL &&
                strcmp(head->value.symbol, "quote") == 0 &&
                tail && tail->type == TYPE_CONS &&
                tail->value.cons.cdr->type == TYPE_NIL) {
                printf("'");
                print_sExpr(tail->value.cons.car);
                return;
            }

            printf("(");
            sExpr *cur = e;
            while (cur->type == TYPE_CONS) {
                print_sExpr(cur->value.cons.car);
                cur = cur->value.cons.cdr;
                if (cur->type == TYPE_CONS) {
                    printf(" ");
                }
            }
            if (cur->type != TYPE_NIL) {
                printf(" . ");
                print_sExpr(cur);
            }
            printf(")");
            break;
        }
    }
}


void free_sExpr(sExpr *e){
    if (!e) return;
    switch (e->type){
        case TYPE_STRING:
            free(e->value.string);
            break;
        case TYPE_SYMBOL:
            free(e->value.symbol);
            break;
        case TYPE_CONS:
            free_sExpr(e->value.cons.car);
            free_sExpr(e->value.cons.cdr);
            break;
        case TYPE_NIL:
            return;
        default:
            break;
    }

    free(e);
}

//Tokenizer

typedef struct {
    char **items;
    int count;
    int pos;
} TokenStream;

TokenStream tokenize(const char* input){
    TokenStream ts;
    ts.count = 0;
    ts.pos = 0;
    int capacity = 16;
    ts.items = malloc(sizeof(char*) * capacity);

    const char *p = input;

    while(*p){

        //Skips Whitespace
        if(isspace(*p)) {p++; continue;}

        if (ts.count >= capacity){
            capacity *= 2;
            ts.items = realloc(ts.items, sizeof(char*) * capacity);
            if (!ts.items) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }

        //Single Character Tokens
        if(*p == '(' || *p == ')'){
            char buf[2] = {*p, '\0'};
            ts.items[ts.count++] = strdup(buf);
            p++;
        } else if(*p == '\'') {
            ts.items[ts.count++] = strdup("'");
            p++;
        }else if (*p== '"'){
            p++;
            const char *start = p;
            while (*p && *p != '"') p++;
            int len = p - start;
            char *tok = (char *)malloc(len + 3);
            tok[0] = '"';
            memcpy(tok + 1, start, len);
            tok[len + 1] = '"';
            tok[len + 2] = '\0';
            ts.items[ts.count++] = tok;
            if (*p == '"') p++; 
        } else {
            //Symbol, Number, or String
            size_t bufsize = 64;
            size_t i = 0;
            char *buf = malloc(bufsize);

            while(*p && !isspace(*p) && *p != '(' && *p != ')'){
                if (i+1 >= bufsize){
                    bufsize *= 2;
                    buf = realloc(buf, bufsize);
                    if (!buf) {
                        fprintf(stderr, "Memory allocation failed\n");
                        exit(1);
                    }
                }
                
                buf[i++] = *p++;
            }
            buf[i] = '\0';
            ts.items[ts.count++] = strdup(buf);
        }
    }
    return ts;
}

void free_tokens(TokenStream *ts){
    for(int i = 0; i < ts->count; i++){
        free(ts->items[i]);
    }
    free(ts->items);
    ts->items = NULL;
    ts->count = ts->pos = 0;
}

char *peek(TokenStream *ts){
    if(ts->pos < ts->count){
        return ts->items[ts->pos];
    }
    return NULL;
}

char *next(TokenStream *ts){
    if(ts->pos < ts->count){
        return ts->items[ts->pos++];
    }
    return NULL;
}

//Parser

sExpr* parse_sexpr(TokenStream *ts);

sExpr* parse_list(TokenStream *ts){
    if (strcmp(peek(ts), ")") == 0) {
        next(ts); // consume ')'
        return NIL;
    }

    sExpr *first = parse_sexpr(ts);
    sExpr *rest = parse_list(ts);
    return cons(first, rest);
};

sExpr *parse_sexpr(TokenStream *ts) {
    const char *tok = peek(ts);
    if (!tok) return NIL; // end of input, return safe NIL

    // Handle list
    if (strcmp(tok, "(") == 0) {
        next(ts); // consume '('
        sExpr *head = NIL;
        sExpr *tail = NIL;

        while ((tok = peek(ts)) && strcmp(tok, ")") != 0) {
            sExpr *elem = parse_sexpr(ts);
            if (elem == NULL) elem = NIL; // safety

            if (head == NIL) {
                head = cons(elem, NIL);
                tail = head;
            } else {
                tail->value.cons.cdr = cons(elem, NIL);
                tail = tail->value.cons.cdr;
            }
        }

        if (!tok) {
            fprintf(stderr, "Parse error: missing closing parenthesis\n");
            return NIL; // unmatched '('
        }

        next(ts); // consume ')'
        return head;
    }

    // Handle quoted expression
    if (strcmp(tok, "'") == 0) {
        next(ts); // consume '
        sExpr *quoted = parse_sexpr(ts);
        sExpr *quote_sym = create_symbol("quote");
        return cons(quote_sym, cons(quoted, NIL));
    }

    // Atom
    next(ts); // consume token
    // Integer
    char *endptr;
    long val = strtol(tok, &endptr, 10);
    if (*endptr == '\0') return create_int(val);

    // Double
    double dval = strtod(tok, &endptr);
    if (*endptr == '\0') return create_double(dval);

    // Symbol
    return create_symbol(tok);
}


sExpr* add(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) {
        return create_symbol("Not a number");
    }

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE) {
        double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
        double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
        return create_double(x + y);
    } else {
        return create_int(a->value.integer + b->value.integer);
    }
}

sExpr* sub(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) {
        return create_symbol("Not a number");
    }

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE) {
        double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
        double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
        return create_double(x - y);
    } else {
        return create_int(a->value.integer - b->value.integer);
    }
}

sExpr* mul(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) {
        return create_symbol("Not a number");
    }

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE) {
        double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
        double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
        return create_double(x * y);
    } else {
        return create_int(a->value.integer * b->value.integer);
    }
}

sExpr* divide(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) {
        return create_symbol("Not a number");
    }

    double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
    if (y == 0) return create_symbol("DivisionByZero");

    double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
    return create_double(x / y);
}

sExpr* mod(sExpr *a, sExpr *b) {
    if (a->type != TYPE_INT || b->type != TYPE_INT) return create_symbol("Not a number");
    if (b->value.integer == 0) return create_symbol("DivisionByZero");
    return create_int(a->value.integer % b->value.integer);
}

sExpr* lt(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) return create_symbol("Not a number");

    double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
    double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
    return (x < y) ? TRUE : NIL;
}

sExpr* gt(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) return create_symbol("Not a number");

    double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
    double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
    return (x > y) ? TRUE : NIL;
}

sExpr* lte(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) return create_symbol("Not a number");

    double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
    double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
    return (x <= y) ? TRUE : NIL;
}

sExpr* gte(sExpr *a, sExpr *b) {
    if ((a->type != TYPE_INT && a->type != TYPE_DOUBLE) ||
        (b->type != TYPE_INT && b->type != TYPE_DOUBLE)) return create_symbol("Not a number");

    double x = (a->type == TYPE_DOUBLE) ? a->value.dbl : a->value.integer;
    double y = (b->type == TYPE_DOUBLE) ? b->value.dbl : b->value.integer;
    return (x >= y) ? TRUE : NIL;
}

sExpr* eq(sExpr *a, sExpr *b) {
    if (a->type != b->type) return NIL;

    switch (a->type) {
        case TYPE_INT:    return (a->value.integer == b->value.integer) ? TRUE : NIL;
        case TYPE_DOUBLE: return (a->value.dbl == b->value.dbl) ? TRUE : NIL;
        case TYPE_STRING: return (strcmp(a->value.string, b->value.string) == 0) ? TRUE : NIL;
        case TYPE_SYMBOL: return (strcmp(a->value.symbol, b->value.symbol) == 0) ? TRUE : NIL;
        case TYPE_NIL:    return TRUE;
        default:        return NIL;
    }
    return NIL;
}

sExpr* not_sExpr(sExpr *a) {
    return (sExpr_to_bool(a)) ? NIL : TRUE;
}

/*
int main(int argc, char *argv[]) {
    FILE *input = NULL;

    // --- Initialize singletons ---
    NIL = malloc(sizeof(sExpr));
    NIL->type = TYPE_NIL;

    TRUE = malloc(sizeof(sExpr));
    TRUE->type = TYPE_SYMBOL;
    TRUE->value.symbol = strdup("t");

    // --- Open input ---
    if (argc == 1) {
        printf("Reading from stdin. Enter S-Expressions (Ctrl+C to quit):\n> ");
        input = stdin;
    } 
    else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "Bad file: %s\n", argv[1]);
            return 1;
        }
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), input)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) {
            if (input == stdin) printf("> ");
            continue;
        }

        // Tokenize
        TokenStream ts = tokenize(buffer);

        // Parse
        sExpr *expr = parse_sexpr(&ts);

        // Print the expression
        print_sExpr(expr);
        printf("\n");

        // Free tokens and expression
        free_tokens(&ts);
        free_sExpr(expr);

        if (input == stdin) printf("> ");
    }

    // --- Close file and cleanup ---
    if (input != stdin) fclose(input);

    free(TRUE->value.symbol);
    free(TRUE);
    free(NIL);

    return 0;
}
*/



