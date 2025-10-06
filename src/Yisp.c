#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sexpr.h"

sExpr *NIL;
sExpr *TRUE;
sExpr *global_env = NULL;

sExpr* create_env(){
    sExpr* frame = cons(NIL, cons(NIL, NIL));
    return cons(frame, NIL);
}

sExpr* set(sExpr* symbol, sExpr* value){
    if (isnil(global_env)) { // Creates fresh env
        global_env = create_env();
    }

    // grabs last frame
    sExpr* env_iter = global_env;
    while (!isnil(cdr(env_iter))) env_iter = cdr(env_iter);
    sExpr* frame = car(env_iter); // gloval frame

    sExpr* symbols = car(frame);
    sExpr* values = car(cdr(frame));

    // Try to update existing binding
    sExpr* sym_it = symbols;
    sExpr* val_it = values;
    while (!isnil(sym_it) && !isnil(val_it)) {
        if (sExpr_to_bool(eq(car(sym_it), symbol))) {
            // update the corresponding value node
            val_it->value.cons.car = value;
            return value;
        }
        sym_it = cdr(sym_it);
        val_it = cdr(val_it);
    }

    
    frame->value.cons.car = cons(symbol, symbols);
    sExpr* values_cons = cdr(frame); 
    values_cons->value.cons.car = cons(value, values);

    return value;
}

sExpr* lookup(sExpr* symbol){
    return lookup_stack(symbol);
}

sExpr* get_symbol(sExpr* target, sExpr* symbol, sExpr* value){
    while (!isnil(symbol) && !isnil(value)) {
        if (sExpr_to_bool(eq(car(symbol), target))) {
            return car(value);
        }
        symbol = cdr(symbol);
        value = cdr(value);
    }
    return create_symbol("undefined");
}

sExpr* push_env(sExpr* params, sExpr* args){
    sExpr* new_frame = cons(params, cons(args, NIL));
    global_env = cons(new_frame, global_env);
    return new_frame;
}

void pop_env(){
    if (isnil(global_env)) return;
    if (isnil(cdr(global_env))) {
        return;
    }
    global_env = cdr(global_env);
}

sExpr* lookup_stack(sExpr* symbol){
    sExpr* env = global_env;
    while(!isnil(env)){
        sExpr* frame = car(env);
        sExpr* val = get_symbol(symbol, car(frame), car(cdr(frame)));
        if(!issymbol(val) || strcmp(val->value.symbol, "undefined") == 0){
            return val;
        }
        env = cdr(env);
    }
    return create_symbol("undefined");
}

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

int isnil(sExpr *e) { return (e->type == TYPE_NIL); }
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
    if (!tok) return NIL; // end of input

    // Handle list
    if (strcmp(tok, "(") == 0) {
        next(ts); // consume
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
    return (a == NIL) ? TRUE : NIL;
}

sExpr* eval(sExpr *expr) {
    if (isnil(expr)) return NIL;

    if (isnumber(expr) || isstring(expr)) {
        return expr;
    }

    if(issymbol(expr)) {
        sExpr* val = lookup_stack(expr);
        if(!issymbol(val) || strcmp(val->value.symbol, "undefined") != 0){
            return val;
        }
        return NIL;
    }

    sExpr *fn = car(expr);
    sExpr *args = cdr(expr);

    sExpr* lambda_expr = NIL;
    const char* sym = NULL;

    if (issymbol(fn)) {
        sym = fn->value.symbol;
        lambda_expr = lookup_stack(fn);
    }else if (fn->type == TYPE_CONS && issymbol(car(fn)) && strcmp(car(fn)->value.symbol, "lambda") == 0){
        lambda_expr = fn;
    }else{
        printf("Invalid function call\n");
        return NIL;
    }


    if (sym != NULL) {
        if (strcmp(sym, "quote") == 0) return car(args);
        if (strcmp(sym, "set") == 0) {
            sExpr *var = car(args);
            sExpr *val = eval(car(cdr(args)));
            return set(var, val);
        }
        if (strcmp(sym, "define") == 0) {
            sExpr* name = car(args);
            sExpr* val_expr = car(cdr(args));
            return set(name, val_expr);
        }
        if (strcmp(sym, "+") == 0) return add(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "-") == 0) return sub(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "*") == 0) return mul(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "/") == 0) return divide(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "%") == 0) return mod(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "<") == 0) return lt(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, ">") == 0) return gt(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "<=") == 0) return lte(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, ">=") == 0) return gte(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "=") == 0) return eq(eval(car(args)), eval(car(cdr(args))));
        if (strcmp(sym, "not") == 0) return not_sExpr(eval(car(args)));
        if (strcmp(sym, "and") == 0) {
            sExpr* a = car(args);
            sExpr* b = car(cdr(args));
            if (isnil(eval(a))) return NIL;
            return eval(b);
        }
        if (strcmp(sym, "or") == 0) {
            sExpr* a = car(args);
            sExpr* b = car(cdr(args));
            if (!isnil(eval(a))) return TRUE;
            return eval(b);
        }
        if (strcmp(sym, "if") == 0) {
            sExpr* cond = car(args);
            sExpr* then_branch = car(cdr(args));
            sExpr* else_branch = car(cdr(cdr(args)));
            if (!isnil(eval(cond))) return eval(then_branch);
            return eval(else_branch);
        }
        if (strcmp(sym, "cond") == 0) {
            sExpr* pair = args;
            while (!isnil(pair)) {
                sExpr* test_expr = car(car(pair));
                sExpr* result_expr = car(cdr(car(pair)));
                if (!isnil(eval(test_expr))) return eval(result_expr);
                pair = cdr(pair);
            }
            return NIL;
        }
    }

     if (!isnil(lambda_expr) && issymbol(car(lambda_expr)) &&
        strcmp(car(lambda_expr)->value.symbol, "lambda") == 0) {

        sExpr* arg_names = car(cdr(lambda_expr));
        sExpr* body = car(cdr(cdr(lambda_expr)));

        // Evaluate arguments
        sExpr* evaled_args = NIL;
        sExpr* tail = NIL;
        sExpr* cur = args;
        while (!isnil(cur)) {
            sExpr* a = eval(car(cur));
            if (isnil(evaled_args)) {
                evaled_args = cons(a, NIL);
                tail = evaled_args;
            } else {
                tail->value.cons.cdr = cons(a, NIL);
                tail = tail->value.cons.cdr;
            }
            cur = cdr(cur);
        }

        // Push environment, evaluate body, pop environment
        push_env(arg_names, evaled_args);
        sExpr* result = eval(body);
        pop_env();
        return result;
    }

    printf("Unknown function: %s\n", sym ? sym : "???");
    return NIL;
}