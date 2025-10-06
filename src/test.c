#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "sexpr.h"

// Counters
int tests_passed = 0;
int tests_failed = 0;

// --- Assertion helpers ---
void assert_int_equal(long expected, sExpr *actual, const char *msg) {
    if (!actual) {
        printf("[FAIL] %s: actual is NULL\n", msg);
        tests_failed++;
        return;
    }
    if ((actual->type == TYPE_INT && actual->value.integer == expected) ||
        (actual->type == TYPE_DOUBLE && actual->value.dbl == expected)) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s: expected %ld, got ", msg, expected);
        print_sExpr(actual);
        printf("\n");
        tests_failed++;
    }
}

void assert_double_equal(double expected, sExpr *actual, const char *msg) {
    if (!actual) {
        printf("[FAIL] %s: actual is NULL\n", msg);
        tests_failed++;
        return;
    }
    double val = (actual->type == TYPE_DOUBLE) ? actual->value.dbl :
                 (actual->type == TYPE_INT ? actual->value.integer : 0.0);
    if (fabs(val - expected) < 1e-6) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s: expected %f, got ", msg, expected);
        print_sExpr(actual);
        printf("\n");
        tests_failed++;
    }
}

int sExpr_equal(sExpr *a, sExpr *b) {
    if (a == b) return 1;        
    if (!a || !b) return 0;
    if (a->type != b->type) return 0;

    switch (a->type) {
        case TYPE_INT:    return a->value.integer == b->value.integer;
        case TYPE_DOUBLE: return fabs(a->value.dbl - b->value.dbl) < 1e-6;
        case TYPE_STRING: return strcmp(a->value.string, b->value.string) == 0;
        case TYPE_SYMBOL: return strcmp(a->value.symbol, b->value.symbol) == 0;
        case TYPE_NIL:    return 1;  // all NILs are equal
        case TYPE_CONS:
            return sExpr_equal(a->value.cons.car, b->value.cons.car) &&
                   sExpr_equal(a->value.cons.cdr, b->value.cons.cdr);
        default: return 0;
    }
}


void assert_sExpr_equal(sExpr *expected, sExpr *actual, const char *msg) {
    if (sExpr_equal(expected, actual)) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s: expected ", msg);
        print_sExpr(expected);
        printf(", got ");
        print_sExpr(actual);
        printf("\n");
        tests_failed++;
    }
}


// --- Tests ---
void test_constructors() {
    printf("=== Constructors ===\n");

    // Integers
    sExpr *i = create_int(42);
    assert_int_equal(42, i, "create_int(42)");

    // Doubles
    sExpr *d = create_double(3.14);
    assert_double_equal(3.14, d, "create_double(3.14)");

    sExpr *dnan = create_double(NAN); // should print "nan"
    print_sExpr(dnan); printf(" [INFO] NaN created\n");

    sExpr *dinf = create_double(INFINITY); // should print "inf"
    print_sExpr(dinf); printf(" [INFO] Inf created\n");

    // Strings
    sExpr *s = create_string("hello");
    assert_sExpr_equal(s, create_string("hello"), "create_string(\"hello\")");

    // Symbols
    sExpr *sym = create_symbol("foo");
    assert_sExpr_equal(sym, create_symbol("foo"), "create_symbol(\"foo\")");

    free_sExpr(d); free_sExpr(dnan); free_sExpr(dinf);
    free_sExpr(s); free_sExpr(sym);
}


void test_cons_and_list() {
    printf("\n=== Cons & List ===\n");

    sExpr *a = create_int(1);
    sExpr *b = create_int(2);
    sExpr *c = create_int(3);

    sExpr *list = cons(a, cons(b, cons(c, NIL)));
    print_sExpr(list); printf("\n");

    assert_sExpr_equal(a, car(list), "car(list) == 1");
    assert_sExpr_equal(cons(b, cons(c, NIL)), cdr(list), "cdr(list) == (2 3)");

    // Nested list
    sExpr *nested = cons(create_int(4), list);
    printf("Nested list: "); print_sExpr(nested); printf("\n");

    // Improper list (not ending in NIL)
    sExpr *improper = cons(create_int(5), create_int(6));
    printf("Improper list: "); print_sExpr(improper); printf("\n");

    free_sExpr(nested);
    free_sExpr(improper);
}


// ------------------- ADD -------------------
void test_add() {
    printf("\n=== test_add ===\n");

    sExpr *i5 = create_int(5);
    sExpr *i3 = create_int(3);
    sExpr *in2 = create_int(-2);
    sExpr *i0 = create_int(0);
    sExpr *d1 = create_double(2.5);

    // Integers
    assert_int_equal(8, add(i5, i3), "5 + 3 = 8");
    assert_int_equal(3, add(i5, in2), "5 + -2 = 3");
    assert_int_equal(5, add(i5, i0), "5 + 0 = 5");

    // Double + Int
    assert_double_equal(7.5, add(i5, d1), "5 + 2.5 = 7.5");

    // Invalid
    sExpr *str = create_string("hi");
    sExpr *badAdd = add(str, i5);
    assert_sExpr_equal(NIL, badAdd, "add(string, int) = NIL");

    free_sExpr(i5); free_sExpr(i3);
    free_sExpr(in2); free_sExpr(i0);
    free_sExpr(d1); free_sExpr(str);
}

// ------------------- SUB -------------------
void test_sub() {
    printf("\n=== test_sub ===\n");

    sExpr *i5 = create_int(5);
    sExpr *i3 = create_int(3);
    sExpr *in2 = create_int(-2);
    sExpr *d1 = create_double(0.5);

    // Integers
    assert_int_equal(2, sub(i5, i3), "5 - 3 = 2");
    assert_int_equal(7, sub(i5, in2), "5 - (-2) = 7");

    // Double + Int
    assert_double_equal(4.5, sub(i5, d1), "5 - 0.5 = 4.5");

    free_sExpr(i5); free_sExpr(i3);
    free_sExpr(in2); free_sExpr(d1);
}

// ------------------- MUL -------------------
void test_mul() {
    printf("\n=== test_mul ===\n");

    sExpr *i5 = create_int(5);
    sExpr *i0 = create_int(0);
    sExpr *in2 = create_int(-2);
    sExpr *d1 = create_double(2.5);

    // Integers
    assert_int_equal(0, mul(i5, i0), "5 * 0 = 0");
    assert_int_equal(-10, mul(i5, in2), "5 * -2 = -10");

    // Double + Int
    assert_double_equal(12.5, mul(i5, d1), "5 * 2.5 = 12.5");

    free_sExpr(i5); free_sExpr(i0);
    free_sExpr(in2); free_sExpr(d1);
}

// ------------------- DIVIDE -------------------
void test_divide() {
    printf("\n=== test_divide ===\n");

    sExpr *i5 = create_int(5);
    sExpr *i3 = create_int(3);
    sExpr *i0 = create_int(0);

    // Normal division
    assert_double_equal(5.0/3.0, divide(i5, i3), "5 / 3 = 1.666...");

    // Divide by zero (must not crash)
    sExpr *div0 = divide(i5, i0);
    assert_sExpr_equal(NIL, div0, "5 / 0 = NIL");

    free_sExpr(i5); free_sExpr(i3); free_sExpr(i0);
}

// ------------------- MOD -------------------
void test_mod() {
    printf("\n=== test_mod ===\n");

    sExpr *i5 = create_int(5);
    sExpr *i3 = create_int(3);
    sExpr *in2 = create_int(-2);
    sExpr *i0 = create_int(0);

    assert_int_equal(2, mod(i5, i3), "5 % 3 = 2");
    assert_int_equal(1, mod(i5, in2), "5 % -2 = 1");

    // Mod by zero
    sExpr *mod0 = mod(i5, i0);
    assert_sExpr_equal(NIL, mod0, "5 % 0 = NIL");

    free_sExpr(i5); free_sExpr(i3);
    free_sExpr(in2); free_sExpr(i0);
}

void test_arithmetic() {
    test_add();
    test_sub();
    test_mul();
    test_divide();
    test_mod();
}

void test_comparisons() {
    printf("\n=== Comparisons ===\n");

    sExpr *i1 = create_int(10);
    sExpr *i2 = create_int(20);
    sExpr *i10 = create_int(10);
    sExpr *d10 = create_double(10.0);
    sExpr *in5 = create_int(-5);

    assert_sExpr_equal(TRUE, lt(i1, i2), "10 < 20");
    assert_sExpr_equal(NIL, gt(i1, i2), "10 > 20");
    assert_sExpr_equal(TRUE, lte(i1, i2), "10 <= 20");
    assert_sExpr_equal(NIL, gte(i1, i2), "10 >= 20");
    assert_sExpr_equal(TRUE, eq(i1, i10), "10 == 10");
    assert_sExpr_equal(TRUE, eq(i1, d10), "10 == 10.0");
    assert_sExpr_equal(TRUE, lt(in5, i1), "-5 < 10");

    free_sExpr(i1); free_sExpr(i2); free_sExpr(i10);
    free_sExpr(d10); free_sExpr(in5);
}


void test_singletons() {
    printf("\n=== Singletons ===\n");

    assert_sExpr_equal(NIL, NIL, "NIL singleton identity");
    assert_sExpr_equal(TRUE, TRUE, "TRUE singleton identity");

    // Truthiness checks
    printf("sExpr_to_bool(NIL) = %d (expected 0)\n", sExpr_to_bool(NIL));
    printf("sExpr_to_bool(TRUE) = %d (expected 1)\n", sExpr_to_bool(TRUE));
    printf("sExpr_to_bool(create_int(0)) = %d (expected 1)\n",
           sExpr_to_bool(create_int(0)));
}


void test_parser() {
    printf("\n=== Parser ===\n");
    const char *inputs[] = {
        "(1 2 3)",
        "(a b c)",
        "'(x y z)",
        "(+ 1 2 3)",
        "((nested (list 1)) 2)",
        "(improper . 42)",   // improper list
        "(missing",          // syntax error
        NULL
    };

    for (int i = 0; inputs[i]; i++) {
        printf("Input: %s\n", inputs[i]);
        TokenStream ts = tokenize(inputs[i]);
        sExpr *expr = parse_sexpr(&ts);
        printf("Parsed: "); print_sExpr(expr); printf("\n");
        free_tokens(&ts);
        free_sExpr(expr);
    }
}

// --- ENVIRONMENT / VARIABLE TESTS ---
void test_define_and_set() {
    printf("\n=== Define & Set! ===\n");

    sExpr *x_sym = create_symbol("x");
    sExpr *val5 = create_int(5);
    sExpr *val10 = create_int(10);

    // define x = 5 via eval
    sExpr *def_expr = cons(create_symbol("define"), cons(x_sym, cons(val5, NIL)));
    eval(def_expr);
    assert_sExpr_equal(val5, lookup(x_sym), "define x = 5");

    // set! x = 10 via eval
    sExpr *set_expr = cons(create_symbol("set"), cons(x_sym, cons(val10, NIL)));
    eval(set_expr);
    assert_sExpr_equal(val10, lookup(x_sym), "set! x = 10");

    free_sExpr(x_sym); free_sExpr(val5); free_sExpr(val10);
}

// --- CONDITIONALS ---
void test_if() {
    printf("\n=== IF ===\n");

    sExpr *cond_true = create_int(1);
    sExpr *cond_false = NIL;
    sExpr *then_val = create_int(42);
    sExpr *else_val = create_int(99);

    // if TRUE then 42 else 99
    sExpr *if_expr_true = cons(create_symbol("if"), cons(cond_true, cons(then_val, cons(else_val, NIL))));
    assert_sExpr_equal(then_val, eval(if_expr_true), "if TRUE -> then");

    // if NIL then 42 else 99
    sExpr *if_expr_false = cons(create_symbol("if"), cons(cond_false, cons(then_val, cons(else_val, NIL))));
    assert_sExpr_equal(else_val, eval(if_expr_false), "if NIL -> else");

    free_sExpr(cond_true); free_sExpr(cond_false); free_sExpr(then_val); free_sExpr(else_val);
}

void test_cond() {
    printf("\n=== COND ===\n");

    sExpr *c1_test = NIL;  // false
    sExpr *c2_test = TRUE;  // true
    sExpr *c1_val = create_int(10);
    sExpr *c2_val = create_int(20);

    sExpr *cond1 = cons(c1_test, cons(c1_val, NIL));
    sExpr *cond2 = cons(c2_test, cons(c2_val, NIL));
    sExpr *cond_expr = cons(create_symbol("cond"), cons(cond1, cons(cond2, NIL)));

    sExpr *res = eval(cond_expr);
    assert_sExpr_equal(c2_val, res, "cond -> first true clause");

    free_sExpr(c1_test); free_sExpr(c2_test); free_sExpr(c1_val); free_sExpr(c2_val);
}

// --- LOGICAL OPERATORS ---
void test_or_and() {
    printf("\n=== OR & AND ===\n");

    // OR tests
    sExpr *or_expr1 = cons(create_symbol("or"), cons(TRUE, cons(NIL, NIL)));
    assert_sExpr_equal(TRUE, eval(or_expr1), "or(TRUE, NIL) -> TRUE");

    sExpr *or_expr2 = cons(create_symbol("or"), cons(NIL, cons(create_int(0), NIL)));
    assert_sExpr_equal(TRUE, eval(or_expr2), "or(NIL, 0) -> TRUE");

    sExpr *or_expr3 = cons(create_symbol("or"), cons(NIL, cons(NIL, NIL)));
    assert_sExpr_equal(NIL, eval(or_expr3), "or(NIL, NIL) -> NIL");

    // AND tests
    sExpr *and_expr1 = cons(create_symbol("and"), cons(TRUE, cons(NIL, NIL)));
    assert_sExpr_equal(NIL, eval(and_expr1), "and(TRUE, NIL) -> NIL");

    sExpr *and_expr2 = cons(create_symbol("and"), cons(TRUE, cons(create_int(1), NIL)));
    assert_sExpr_equal(create_int(1), eval(and_expr2), "and(TRUE, 1) -> 1");

    sExpr *and_expr3 = cons(create_symbol("and"), cons(NIL, cons(create_int(1), NIL)));
    assert_sExpr_equal(NIL, eval(and_expr3), "and(NIL, 1) -> NIL");
}

int main() {
    // Initialize singletons
    NIL = malloc(sizeof(sExpr));
    NIL->type = TYPE_NIL;

    TRUE = malloc(sizeof(sExpr));
    TRUE->type = TYPE_SYMBOL;
    TRUE->value.symbol = strdup("t");

    global_env = create_env();

    test_constructors();
    test_cons_and_list();
    test_arithmetic();
    test_comparisons();
    test_singletons();
    test_parser();
    test_define_and_set();
    test_if();
    test_cond();
    test_or_and();


    printf("\n=== Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    free(TRUE->value.symbol);
    free(TRUE);
    free(NIL);
    return 0;
}
