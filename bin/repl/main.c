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

  expr_t *expr = parser_parse_expression(&parser, input);
  free(input);

  if (parser.had_errors) {
    free(expr);
    return NULL;
  }

  return expr;
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
