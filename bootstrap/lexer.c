#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "lang.h"

static void lexer_error(lexer_t *lexer, char *msg, ...) {
  va_list args;
  va_start(args, msg);
  error(lexer->file, lexer->line, msg, args);
  va_end(args);
}

int lexer_create(lexer_t *lexer, char *file, char *input) {
  lexer->ptr = input;
  lexer->file = file;
  lexer->line = 1;

  return 0;
}

static int lexer_read_identifier(lexer_t *lexer, char **value) {
  char *start = lexer->ptr;

  if (!(*lexer->ptr >= 'a' && *lexer->ptr <= 'z')) {
    lexer_error(lexer, "Expected identifier");
    return 1;
  }

  for (;;) {
    lexer->ptr++;

    if (*lexer->ptr >= 'a' && *lexer->ptr <= 'z') {
      continue;
    } else {
      break;
    }
  }

  // We're pointing to the first character that's not part of the
  // identifier
  *value = strndup(start, lexer->ptr - start);
  return 0;
}

static int lexer_read_string(lexer_t *lexer, char **value) {
  if (*lexer->ptr != '"') {
    lexer_error(lexer, "Expected start of string");
    return 1;
  }

  char *start = ++lexer->ptr;

  while (*lexer->ptr++ != '"');

  // We're pointing to the first character that's not part of the
  // identifier
  *value = strndup(start, lexer->ptr - start - 1);
  return 0;
}

static int lexer_read_number(lexer_t *lexer, int *number) {
  *number = 0;

  while (*lexer->ptr >= '0' && *lexer->ptr <= '9') {
    int digit = *lexer->ptr++ - '0';
    *number = (*number * 10) + digit;
  }

  return 0;
}

int lexer_next(lexer_t *lexer, token_t *token) {
  // Skip whitespace
  while (*lexer->ptr == ' ' || *lexer->ptr == '\t') {
    lexer->ptr++;
  }

  char c = *lexer->ptr;
  switch (c) {
  case 0:
    token->type = TOKEN_EOF;
    break;

  case 'a' ... 'z':
    token->type = TOKEN_ID;
    if (lexer_read_identifier(lexer, &token->value_id)) {
      return 1;
    }
    break;

  case '0' ... '9':
    token->type = TOKEN_NUMBER;
    if (lexer_read_number(lexer, &token->value_number)) {
      return 1;
    }
    break;

  case ',':
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '/':
    token->type = c;
    lexer->ptr++;
    break;

  case '\n':
    token->type = TOKEN_NEWLINE;
    lexer->ptr++;
    lexer->line++;
    break;

  case '"':
    token->type = TOKEN_STRING;
    if (lexer_read_string(lexer, &token->value_string)) {
      return 1;
    }
    break;

  default:
    lexer_error(lexer, "Unexpected character '%c'", c);
    return 1;
  }

  return 0;
}
