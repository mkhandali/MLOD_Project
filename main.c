#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>



int main(int argc, char** argv) {

  puts("Lusp Version 0.1");
  puts("Appuyer sur Ctrl+c pour sortir du shell\n");

  while (1) {

    /* Output our prompt */
	char* input = readline("lispy> ");

    /* Read a line of user input of maximum size 2048 */
	add_history(input);

    /* Echo input back to user */
    printf("output: %s\n", input);

    free(input);
  }

  return 0;
}
