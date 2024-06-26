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

static void repl_parse_eval_and_print(char *input, object *scope) {
  parser_t parser;
  if (parser_init(&parser, "repl", input) != 0) {
    goto end;
  }

  expr_t *expr;
  object *obj = NULL;

  while ((expr = parser_next(&parser))) {
    if (expr) {
      expr_dump(expr);
    }

    obj = eval(expr, scope);
    send(send(obj, intern("inspect")), intern("println"));
  }

 end:
  free(input);
}

static void repl(object *scope) {
  char *input;

  object *hw = string_new("DAS//compute REPL.\nWrite 'quit' to exit.");
  send(hw, intern("println"));

  while (1) {
    printf("> ");

    input = repl_read();
    if (!input) {
      break;
    }

    repl_parse_eval_and_print(input, scope);
  }
}

static char *read_entire_file(char *file) {
  FILE *f = fopen(file, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  char *body = malloc(fsize + 1);
  fread(body, fsize, 1, f);
  fclose(f);

  body[fsize] = 0;
  return body;
}

static void process_file(char *file, object *scope) {
  char *body = read_entire_file(file);
  parser_t parser;
  expr_t *expr;

  if (parser_init(&parser, file, body) != 0) {
    return;
  }

  while ((expr = parser_next(&parser))) {
    eval(expr, scope);
  }

  free(body);
}

int main(int argc, char **argv) {
  object *root_scope = root_scope_bootstrap();

  if (argc > 1) {
    process_file(argv[1], root_scope);
  } else {
    repl(root_scope);
  }

  return 0;
}
