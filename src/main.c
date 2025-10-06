#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sexpr.h"

extern sExpr *NIL;
extern sExpr *TRUE;
extern sExpr *global_env;

int main(int argc, char *argv[]) {
    FILE *input = NULL;

    NIL = malloc(sizeof(sExpr));
    NIL->type = TYPE_NIL;

    TRUE = create_symbol("t");
    set(TRUE, TRUE);

    global_env = create_env();

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

        TokenStream ts = tokenize(buffer);
        sExpr *expr = parse_sexpr(&ts);
        sExpr *result = eval(expr);

        print_sExpr(result);
        printf("\n");

        if (input == stdin) printf("> ");

        free_tokens(&ts);
    }

    if (input != stdin) fclose(input);

    free(TRUE->value.symbol);
    free(TRUE);
    free(NIL);

    return 0;
}
