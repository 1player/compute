#include <stdio.h>
#include "common.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  char *s = "(1 + 2) * 3 - a\n1 +";

  parser_t parser;
  toplevel_t *top;

  if (parser_create(&parser, "example", s) != 0) {
    printf("Unable to create parser\n");
    return 1;
  }

  top = parser_parse(&parser);

  if (!parser.had_errors) {
    for (int i = 0; i < top->exprs.size; i++) {
      fprintf(stderr, "Expression %d: ", i);
      expr_dump(top->exprs.elements[i]);
      fputc('\n', stderr);
    }
  }
}
