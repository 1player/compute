#include <stdio.h>
#include <string.h>

#include "lang.h"
#include "object.h"

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

expr_t *repl_parse(char *input) {
  parser_t parser;

  expr_t *expr = parser_parse_expression(&parser, input);
  free(input);

  if (parser.had_errors) {
    free(expr);
    return NULL;
  }

  return expr;
}

expr_t *repl_eval(expr_t *expr) {
  return expr;
}


void repl_print(expr_t *expr) {
  expr_dump(expr);
  free(expr);

  putchar('\n');
}

void repl() {
  char *input;
  expr_t *expr, *result;

  printf("DAS//compute REPL.\nWrite 'quit' to exit.\n");

  while (1) {
    printf("> ");
    input = repl_read();
    if (!input) {
      break;
    }
    expr = repl_parse(input);
    if (!expr) {
      continue;
    }

    result = repl_eval(expr);
    if (!result) {
      continue;
    }
    repl_print(expr);
  }
}

int main() {
  //repl();

  World world;
  world_bootstrap(&world);

  return 0;
}
