#include <stdio.h>
#include <string.h>
#include "lang/lang.h"

char *repl_read() {
  char *line = NULL;
  size_t n = 0;

  if (getline(&line, &n, stdin) == -1) {
    return NULL;
  }
  if (strcmp(line, "quit\n") == 0 || strcmp(line, "q\n") == 0) {
    return NULL;
  }

  return line;
}

expr_t *repl_eval(char *input) {
  parser_t parser;

  if (parser_create(&parser, "repl", input) != 0) {
    panic("Unable to create parser\n");
  }

  toplevel_t *top = parser_parse(&parser);

  free(input);

  if (!parser.had_errors && top->exprs.size > 0) {
    return top->exprs.elements[0];
  }

  return NULL;
}

void repl_print(expr_t *expr) {
  expr_dump(expr);
  free(expr);

  putchar('\n');
}

int main() {
  char *input;
  expr_t *expr;

  printf("DAS//compute REPL.\nWrite 'quit' to exit.\n");

  while (1) {
    printf("> ");
    input = repl_read();
    if (!input) {
      break;
    }
    expr = repl_eval(input);
    if (expr) {
      repl_print(expr);
    }
  }

  return 0;
}
