#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "object.h"

static char *repl_read() {
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

static expr_t *repl_parse(char *input) {
  parser_t parser;

  expr_t *expr = parser_parse_expression(&parser, input);
  free(input);

  if (parser.had_errors) {
    free(expr);
    return NULL;
  }

  expr_dump(expr);
  return expr;
}

static Object *repl_eval(expr_t *expr) {
  return eval(expr);
}

static void repl_print(Object *obj) {
  send(send(obj, "inspect"), "println");
}

static void repl() {
  char *input;
  expr_t *expr;
  Object *result;

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
    repl_print(result);
  }
}

int main() {
  world_bootstrap();

  Object *hw = world_make_string("DAS//compute REPL.\nWrite 'quit' to exit.");
  send(hw, "println");

  repl();

  return 0;
}
