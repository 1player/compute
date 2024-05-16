#include <stdio.h>
#include "common.h"

int main(int argc, char **argv) {
  char *s = "print(\"hello\")\nprint((1 + 2) * 3)";

  lexer_t lexer;
  token_t token;
  lexer_create(&lexer, s);

  while (lexer_next(&lexer, &token) == 0) {
    if (token.type == TOKEN_EOF)
      break;

    switch (token.type) {
    case TOKEN_ID:
      printf("ID '%s'\n", token.value_id);
      break;
    case TOKEN_STRING:
      printf("String '%s'\n", token.value_string);
      break;
    case TOKEN_NUMBER:
      printf("Number '%d'\n", token.value_number);
      break;
    default:
      printf("Character '%c'\n", token.type);
    }
  }
}
