#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "object.h"
#include "builtins.h"

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
  expr_t *expr = parse_expression(input);
  free(input);

  if (expr) {
    expr_dump(expr);
  }

  return expr;
}

static Object *repl_eval(expr_t *expr, Object *scope) {
  return eval(expr, scope);
}

static void repl_print(Object *obj) {
  send(send(obj, "inspect"), "println");
}

static void repl(Object *scope) {
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

    result = repl_eval(expr, scope);
    repl_print(result);
  }
}

int main() {
  Object *global_scope = bootstrap();

  Object *hw = string_new("DAS//compute REPL.\nWrite 'quit' to exit.");
  send(hw, "println");

  repl(global_scope);

  return 0;
}
