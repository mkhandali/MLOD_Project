//gcc -std=c99 -Wall main.c mpc.c -ledit -lm -o main




#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h" // parser lib

#include <editline/readline.h>
#include <editline/history.h>



long eval_op(long x, char* op, long y) {
  if (!strcmp(op, "+")) return x + y;
  if (!strcmp(op, "-")) return x - y;
  if (!strcmp(op, "*")) return x * y;
  if (!strcmp(op, "/")) return x / y;
  if (!strcmp(op, "%")) return x % y;
  if (!strcmp(op, "^")) return pow(x,y);
  if (!strcmp(op, "min")) return (x<y ? x : y);
  if (!strcmp(op, "max")) return (x<y ? y : x);
  return 0;
}

long eval(mpc_ast_t* t) {

  /* If tagged as number return it directly. */
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  /* The operator is always second child. */
  char* op = t->children[1]->contents;

  /* We store the third child in `x` */
  long x = eval(t->children[2]);

  /* Iterate the remaining children and combining. */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}


void parsing(char* line,mpc_parser_t* Zezay){

    mpc_result_t r;

    if (mpc_parse("<stdin>", line, Zezay, &r)) {
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);

    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
}

int main(int argc, char** argv) {

  /* Create Some Parsers */
  mpc_parser_t* Valeur   = mpc_new("number");
  mpc_parser_t* Operateur = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Zezay    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\" ; \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Valeur, Operateur, Expr, Zezay);

  puts("Zezay Version 0.0.0.0.2");
  puts("Appuyez sur Ctrl+c pour sortir\n");



  while (1) {

	char* input = readline("zezay> ");
	add_history(input);

	parsing(input,Zezay);

    free(input);
  }

  /* Undefine and delete our parsers */
  mpc_cleanup(4, Valeur, Operateur, Expr, Zezay);

  return 0;
}
