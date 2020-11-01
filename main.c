

#include "mpc.h"

#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif


enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR };

typedef struct value {
  int type;
  long num;
  char* err;
  char* sym;
  int count;
  struct value** cell;
} value;


value* setNum(long x) {
  value* v = malloc(sizeof(value));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}


value* setErr(char* m) {
  value* v = malloc(sizeof(value));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}


value* setSym(char* s) {
  value* v = malloc(sizeof(value));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}


value* setSexpr(void) {
  value* v = malloc(sizeof(value));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void freeValue(value* v) {

  switch (v->type) {

    case LVAL_NUM: break;
    

    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;
    

    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        freeValue(v->cell[i]);
      }

      free(v->cell);
    break;
  }
  

  free(v);
}

value* addValue(value* v, value* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(value*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

value* popValue(value* v, int i) {

  value* x = v->cell[i];
  

  memmove(&v->cell[i], &v->cell[i+1],
    sizeof(value*) * (v->count-i-1));
  

  v->count--;
  

  v->cell = realloc(v->cell, sizeof(value*) * v->count);
  return x;
}

value* takeValue(value* v, int i) {
  value* x = popValue(v, i);
  freeValue(v);
  return x;
}

void printValue(value* v);

void printSexpr(value* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    
    printValue(v->cell[i]);
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void printValue(value* v) {
  switch (v->type) {
    case LVAL_NUM:   printf("%li", v->num); break;
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR: printSexpr(v, '(', ')'); break;
  }
}

void printlnValue(value* v) { printValue(v); putchar('\n'); }

value* builtinOp(value* a, char* op) {
  
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      freeValue(a);
      return setErr("Cannot operate on non-number!");
    }
  }
  
  value* x = popValue(a, 0);
  
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }
  
  while (a->count > 0) {
  
    value* y = popValue(a, 0);
    
    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        freeValue(x); freeValue(y);
        x = setErr("Division By Zero.");
        break;
      }
      x->num /= y->num;
    }
    
    freeValue(y);
  }

  
  freeValue(a);
  return x;
}

value* evalValue(value* v);

value* evalSexpr(value* v) {
  

  for (int i = 0; i < v->count; i++) {
    v->cell[i] = evalValue(v->cell[i]);
  }
  

  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return takeValue(v, i); }
  }
  

  if (v->count == 0) { return v; }
  
  if (v->count == 1) { return takeValue(v, 0); }
  
  value* f = popValue(v, 0);
  if (f->type != LVAL_SYM) {
    freeValue(f); freeValue(v);
    return setErr("S-expression Does not start with symbol.");
  }
  

  value* result = builtinOp(v, f->sym);
  freeValue(f);
  return result;
}

value* evalValue(value* v) {
  if (v->type == LVAL_SEXPR) { return evalSexpr(v); }
  return v;
}

value* readNum(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    setNum(x) : setErr("invalid number");
}

value* readValue(mpc_ast_t* t) {
  
  if (strstr(t->tag, "number")) { return readNum(t); }
  if (strstr(t->tag, "symbol")) { return setSym(t->contents); }
  
  value* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = setSexpr(); } 
  if (strstr(t->tag, "sexpr"))  { x = setSexpr(); }
  
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = addValue(x, readValue(t->children[i]));
  }
  
  return x;
}

int main(int argc, char** argv) {
  
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr  = mpc_new("sexpr");
  mpc_parser_t* Expr   = mpc_new("expr");
  mpc_parser_t* Lispy  = mpc_new("lispy");
  
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                          \
      number : /-?[0-9]+/ ;                    \
      symbol : '+' | '-' | '*' | '/' ;         \
      sexpr  : '(' <expr>* ')' ;               \
      expr   : <number> | <symbol> | <sexpr> ; \
      lispy  : /^/ <expr>* /$/ ;               \
    ",
    Number, Symbol, Sexpr, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.5");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
  
    char* input = readline("lispy> ");
    add_history(input);
    
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      value* x = evalValue(readValue(r.output));
      printlnValue(x);
      freeValue(x);
      mpc_ast_delete(r.output);
    } else {    
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
    
  }
  
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
  
  return 0;
}

