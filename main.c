//gcc -std=c99 -Wall main.c mpc.c -ledit -lm -o main

// Tutoriel: http://www.buildyourownlisp.com


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h" // parser lib

#include <editline/readline.h>
#include <editline/history.h>

 ///////////////////////////////STRUCT/////////////////////////////////

/* Create Enumeration of Possible Error Types */
enum typeErr{ LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };
enum type{ LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

typedef struct
   {
   enum type type;
   int count;
   union
      {
	   long num;
	   char* err;
	   char* sym;
	   struct value** cell;
      } u;
   } value;



   //////////////////////////////////////////// CREATOR ////////////////////////
value* setNum(long x) {
	  value* v = malloc(sizeof(value));
	  v->type = LVAL_NUM;
	  v->u.num = x;
	  return v;
}

value* setErr(char* e) {
	  value* v = malloc(sizeof(value));
	  v->type = LVAL_ERR;
	  v->u.err = (char*)malloc(strlen(e) + 1);
	  strcpy(v->u.err, e);
	  return v;
}

/* Construct a pointer to a new Symbol lval */
value* setSym(char* s) {
  value* v = malloc(sizeof(value));
  v->type = LVAL_SYM;
  v->u.sym = (char*)malloc(strlen(s) + 1);
  strcpy(v->u.sym, s);
  return v;
}

/* A pointer to a new empty Sexpr lval */
value* setSexpr(void) {
  value* v = malloc(sizeof(value));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->u.cell = NULL;
  return v;
}

//////////////////////////////////////////////// DESTRUCTOR ///////////////////////////
void freeValue(value* v) {

  switch (v->type) {
    /* Do nothing special for number type */
    case LVAL_NUM: break;

    /* For Err or Sym free the string data */
    case LVAL_ERR: free(v->u.err); break;
    case LVAL_SYM: free(v->u.sym); break;

    /* If Sexpr then delete all elements inside */
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        freeValue(v->u.cell[i]);
      }
      /* Also free the memory allocated to contain the pointers */
      free(v->u.cell);
    break;
  }

  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

////////////////////////////////////////////////// READER /////////////////////////////////

value* readNum(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    setNum(x) : setErr("invalid number");
}

value* read(mpc_ast_t* t) {

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return readNum(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  /* If root (>) or sexpr then create empty list */
  value* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = addToSexpr(x, read(t->children[i]));
  }

  return x;
}

value* addToSexpr(value* v, value* x) {
  v->count++;
  v->u.cell = realloc(v->u.cell, sizeof(value*) * v->count);
  v->u.cell[v->count-1] = x;
  return v;
}



//////////////////////////////////////////// PRINTER ////////////////////////////////

void printSexpr(value* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {

    /* Print Value contained within */
    printValue(v->u.cell[i]);

    /* Don't print trailing space if last element */
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void printValue(value* v) {
  switch (v->type) {
    case LVAL_NUM:   printf("%li", v->u.num); break;
    case LVAL_ERR:   printf("Error: %s", v->u.err); break;
    case LVAL_SYM:   printf("%s", v->u.sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  }
}

void printlnValue(value* v) { printvalue(v); putchar('\n'); }


/////////////////////////////////////////////////// EVALUATION ////////////////////////
/*
value eval_op(value x, char* op, value y) {
  if (!strcmp(op, "+")) return setNum(x.u.num + y.u.num);
  if (!strcmp(op, "-")) return setNum(x.u.num - y.u.num);
  if (!strcmp(op, "*")) return setNum(x.u.num * y.u.num);
  if (!strcmp(op, "^")) return setNum(pow(x.u.num,y.u.num));
  if (!strcmp(op, "min")) return (x.u.num<y.u.num ? x : y);
  if (!strcmp(op, "max")) return (x.u.num<y.u.num ? y : x);
  if (!strcmp(op, "/")) return (!y.u.num ? setErr(LERR_DIV_ZERO) : setNum(x.u.num / y.u.num));
  if (!strcmp(op, "%")) return (!y.u.num ? setErr(LERR_DIV_ZERO) : setNum(x.u.num % y.u.num));

  return setErr(LERR_BAD_OP);
}
*/




value* popValue(value* v, int i) {
  /* Find the item at "i" */
  value* x = v->u.cell[i];

  /* Shift memory after the item at "i" over the top */
  memmove(&v->u.cell[i], &v->u.cell[i+1],
    sizeof(value*) * (v->count-i-1));

  /* Decrease the count of items in the list */
  v->count--;

  /* Reallocate the memory used */
  v->u.cell = realloc(v->u.cell, sizeof(value*) * v->count);
  return x;
}

value* takeValue(value* v, int i) {
  value* x = popValue(v, i);
  lval_del(v);
  return x;
}

value* evalSexpr(value* v) {

  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->u.cell[i] = eval(v->u.cell[i]);
  }

  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
	value* v2=v->u.cell[i];
    if (v2->type == LVAL_ERR) { return takeValue(v, i); }
  }

  /* Empty Expression */
  if (v->count == 0) { return v; }

  /* Single Expression */
  if (v->count == 1) { return takeValue(v, 0); }

  /* Ensure First Element is Symbol */
  value* f = popValue(v, 0);
  if (f->type != LVAL_SYM) {
	freeValue(f);
	freeValue(v);
    return setErr("S-expression Does not start with symbol!");
  }

  /* Call builtin with operator */
  value* result = builtinOp(v, f->u.sym);
  lval_del(f);
  return result;
}

value* eval(value* v) {
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) { return evalSexpr(v); }
  /* All other lval types remain the same */
  return v;
}




/*
value eval(mpc_ast_t* t) {

  if (strstr(t->tag, "number")) {
	errno = 0; // Variable spéciale detection d'erreur
	long result = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? setNum(result) : setErr(LERR_BAD_NUM);
  }

  char* op = t->children[1]->contents;

  value result = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    result = eval_op(result, op, eval(t->children[i]));
    i++;
  }

  return result;
}


void parsing(char* line,mpc_parser_t* Zezay){

    mpc_result_t r;

    if (mpc_parse("<stdin>", line, Zezay, &r)) {
      value result = eval(r.output);
      printValue(result);
      mpc_ast_delete(r.output);

    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
}
*/

int main(int argc, char** argv) {

  /* Create Some Parsers */
  mpc_parser_t* Valeur   = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr  = mpc_new("sexpr");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Zezay    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      symbol : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\" ; \
	  sexpr  : '(' <expr>* ')' ;               \
      expr     : <number> | '(' <symbol> <sexpr>+ ')' ;  \
      lispy    : /^/ <symbol> <expr>+ /$/ ;             \
    ",
    Valeur, Symbol, Sexpr, Expr, Zezay);

  puts("Zezay Version 0.0.0.0.2");
  puts("Appuyez sur Ctrl+c pour sortir\n");



  while (1) {

	char* input = readline("zezay> ");
	add_history(input);

	parsing(input,Zezay);

    free(input);
  }

  /* Undefine and delete our parsers */
  mpc_cleanup(4, Valeur, Symbol, Sexpr, Expr, Zezay);

  return 0;
}
