

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

#define LASSERT(args, cond, err) \
		if (!(cond)) { freeValue(args); return setErr(err); }

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR,  LVAL_QEXPR };

typedef struct value {
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct value** cell;
} value;


value* evalValue(value* v);

/////////////////////////////////////////// CONSTRUCTORS /////////

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

value* setQexpr(void) {
	value* v = malloc(sizeof(value));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

/////////////////////////////////////////// DESTRUCTORS /////////

void freeValue(value* v) {

	switch (v->type) {

	case LVAL_NUM: break;


	case LVAL_ERR: free(v->err); break;
	case LVAL_SYM: free(v->sym); break;

	case LVAL_QEXPR:
	case LVAL_SEXPR:
		for (int i = 0; i < v->count; i++) {
			freeValue(v->cell[i]);
		}

		free(v->cell);
		break;
	}


	free(v);
}


///////////////////////////////////////// MEMORY MANAGEMENT /////////////////////

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


///////////////////////////PRINTERS //////////////////////////

void printValue(value* v);

void printExpr(value* v, char open, char close) {
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
	case LVAL_SEXPR: printExpr(v, '(', ')'); break;
	case LVAL_QEXPR: printExpr(v, '{', '}'); break;
	}
}

void printlnValue(value* v) { printValue(v); putchar('\n'); }


//////////////////////////////// SYMBOL EVALUATION  /////////////////////////////

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

value* builtinHead(value* a) {
	/* Check Error Conditions */
	if (a->count != 1) {
		freeValue(a);
		return setErr("Function 'head' passed too many arguments!");
	}

	if (a->cell[0]->type != LVAL_QEXPR) {
		freeValue(a);
		return setErr("Function 'head' passed incorrect types!");
	}

	if (a->cell[0]->count == 0) {
		freeValue(a);
		return setErr("Function 'head' passed {}!");
	}

	/* Otherwise take first argument */
	value* v = takeValue(a, 0);

	/* Delete all elements that are not head and return */
	while (v->count > 1) { freeValue(popValue(v, 1)); }
	return v;
}

value* builtinTail(value* a) {
	/* Check Error Conditions */
	if (a->count != 1) {
		freeValue(a);
		return setErr("Function 'tail' passed too many arguments!");
	}

	if (a->cell[0]->type != LVAL_QEXPR) {
		freeValue(a);
		return setErr("Function 'tail' passed incorrect types!");
	}

	if (a->cell[0]->count == 0) {
		freeValue(a);
		return setErr("Function 'tail' passed {}!");
	}

	/* Take first argument */
	value* v = takeValue(a, 0);

	/* Delete first element and return */
	freeValue(popValue(v, 0));
	return v;
}

value* builtinList(value* a) {
	a->type = LVAL_QEXPR;
	return a;
}

value* builtinEval(value* a) {

	if (a->count == 1) {
		freeValue(a);
		return setErr("Function 'eval' passed too many arguments!");
	}
	if (a->cell[0]->type == LVAL_QEXPR) {
		freeValue(a);
		return setErr("Function 'eval' passed incorrect type!");
	}

	value* x = takeValue(a, 0);
	x->type = LVAL_SEXPR;
	return evalValue(x);
}

value* joinValue(value* x, value* y) {

	/* For each cell in 'y' add it to 'x' */
	while (y->count) {
		x = addValue(x, popValue(y, 0));
	}

	/* Delete the empty 'y' and return 'x' */
	freeValue(y);
	return x;
}

value* builtinJoin(value* a) {

	for (int i = 0; i < a->count; i++) {

		if ( a->cell[i]->type == LVAL_QEXPR) {
			freeValue(a);
			return setErr("Function 'join' passed incorrect type.");
		}
	}

	value* x = popValue(a, 0);

	while (a->count) {
		x = joinValue(x, popValue(a, 0));
	}

	freeValue(a);
	return x;
}



value* builtin(value* a, char* func) {
  if (strcmp("list", func) == 0) { return builtinList(a); }
  if (strcmp("head", func) == 0) { return builtinHead(a); }
  if (strcmp("tail", func) == 0) { return builtinTail(a); }
  if (strcmp("join", func) == 0) { return builtinJoin(a); }
  if (strcmp("eval", func) == 0) { return builtinEval(a); }
  if (strstr("+-/*", func)) { return builtinOp(a, func); }
  freeValue(a);
  return setErr("Unknown Function!");
}

////////////////////////////////////////// EVALUATION /////////////////////////


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
    freeValue(f);
    freeValue(v);
    return setErr("S-expression Does not start with symbol.");
  }

  /* Call builtin with operator */
  value* result = builtin(v, f->sym);
  freeValue(f);
  return result;
}


value* evalValue(value* v) {
	if (v->type == LVAL_SEXPR) { return evalSexpr(v); }
	return v;
}


////////////////////////////////////////////////// READERS //////////////////////////

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
	if (strstr(t->tag, "qexpr"))  { x = setQexpr(); }

	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
		x = addValue(x, readValue(t->children[i]));
	}

	return x;
}

int main(int argc, char** argv) {

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr  = mpc_new("sexpr");
	mpc_parser_t* Qexpr  = mpc_new("qexpr");
	mpc_parser_t* Expr   = mpc_new("expr");
	mpc_parser_t* Lispy  = mpc_new("lispy");

	  mpca_lang(MPCA_LANG_DEFAULT,
	    "                                                    \
	      number : /-?[0-9]+/ ;                              \
	      symbol : \"list\" | \"head\" | \"tail\" | \"eval\" \
	             | \"join\" | '+' | '-' | '*' | '/' ;        \
	      sexpr  : '(' <expr>* ')' ;                         \
	      qexpr  : '{' <expr>* '}' ;                         \
	      expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
	      lispy  : /^/ <expr>* /$/ ;                         \
	    ",
	    Number, Symbol, Sexpr, Qexpr, Expr, Lispy);


	puts("Luluspy Version 0.10");
	puts("Appuyez sur Ctrl+c pour sortir\n");

	while (1) {

		char* input = readline("luluspy> ");
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

