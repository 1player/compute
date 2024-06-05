#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "lang.h"

static struct {
  char *keyword;
  enum token_type type;
} keyword_table[] = {
  { "if",   TOKEN_IF },
  { "else", TOKEN_ELSE },
  { "func", TOKEN_FUNC },
  { "loop", TOKEN_LOOP },
  { "break", TOKEN_BREAK },
  { "continue", TOKEN_CONTINUE },
  { "scope", TOKEN_SCOPE },
  { "self", TOKEN_SELF },
};

static const int n_keywords = sizeof(keyword_table) / sizeof(keyword_table[0]);

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

static int match_keyword(char *word, size_t word_len, token_t *token) {
  for (int i = 0; i < n_keywords; i++) {
    char *keyword = keyword_table[i].keyword;
    if (strequals(keyword, strlen(keyword), word, word_len)) {
      token->type = keyword_table[i].type;
      return 1;
    }
  }

  return 0;
}

static void lexer_read_word(lexer_t *lexer, token_t *token) {
  char *word = lexer->ptr;

  for (;;) {
    char c = *++lexer->ptr;

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || (c == '_')) {
      continue;
    } else {
      break;
    }
  }

  size_t word_len = lexer->ptr - word;
  if (match_keyword(word, word_len, token)) {
    return;
  }

  token->type = TOKEN_ID;
  token->value_id = strndup(word, word_len);
  return;
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


char peek_at(lexer_t *lexer, int ahead) {
  char *ptr = lexer->ptr;
  while (ahead-- > 0 && *ptr != '\0') {
    ptr++;
  }

  return *ptr;
}

char peek(lexer_t *lexer) {
  return peek_at(lexer, 0);
}

void advance(lexer_t *lexer) {
  if (*lexer->ptr == '\0') {
    return;
  }

  if (*lexer->ptr == '\n') {
    lexer->line++;
  }
  lexer->ptr++;
}

void lexer_skip_comment(lexer_t *lexer) {
  char c;
  do {
    c = peek(lexer);
    advance(lexer);
  } while(c != '\0' && c != '\n');
}

char *lexer_explain(token_t *tok) {
  char *e = NULL;

  if ((int)tok->type > 0 && (int)tok->type < 256) {
    asprintf(&e, "%c", (unsigned char)tok->type);
    return e;
  }

  // TODO: lookup the keyword in the keyword table
  switch (tok->type) {
  case TOKEN_EOF:
    asprintf(&e, "EOF");
    break;

  case TOKEN_NEWLINE:
    asprintf(&e, "newline");
    break;

  case TOKEN_ID:
    asprintf(&e, "identifier '%s'", tok->value_id);
    break;

  case TOKEN_STRING:
    asprintf(&e, "string '%s'", tok->value_string);
    break;

  case TOKEN_NUMBER:
    asprintf(&e, "number '%d'", tok->value_number);
    break;

  case TOKEN_DEFINE:
    asprintf(&e, ":=");
    break;

  case TOKEN_EQUALS:
    asprintf(&e, "==");
    break;

  case TOKEN_IS:
    asprintf(&e, "===");
    break;

  case TOKEN_GTE:
    asprintf(&e, ">=");
    break;

  case TOKEN_LTE:
    asprintf(&e, "<=");
    break;

  case TOKEN_IF:
    asprintf(&e, "if");
    break;

  case TOKEN_ELSE:
    asprintf(&e, "else");
    break;

  case TOKEN_FUNC:
    asprintf(&e, "func");
    break;

  case TOKEN_LOOP:
    asprintf(&e, "loop");
    break;

  case TOKEN_BREAK:
    asprintf(&e, "break");
    break;

  case TOKEN_CONTINUE:
    asprintf(&e, "continue");
    break;

  case TOKEN_SCOPE:
    asprintf(&e, "scope");
    break;

  case TOKEN_SELF:
    asprintf(&e, "self");
    break;
  }

  return e;
}

int lexer_next(lexer_t *lexer, token_t *token) {
 restart:
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
  case 'A' ... 'Z':
  case '_':
    lexer_read_word(lexer, token);
    break;

  case '0' ... '9':
    token->type = TOKEN_NUMBER;
    if (lexer_read_number(lexer, &token->value_number)) {
      return 1;
    }
    break;

  case '=':
    if (peek_at(lexer, 1) == '=') {
      if (peek_at(lexer, 2) == '=') {
        token->type = TOKEN_IS;
        lexer->ptr += 3;
      } else {
        token->type = TOKEN_EQUALS;
        lexer->ptr += 2;
      }
    } else {
      token->type = c;
      lexer->ptr++;
    }
    break;

  case '/':
    if (peek_at(lexer, 1) == '/') {
      lexer->ptr += 2;
      lexer_skip_comment(lexer);
      goto restart;
    }

    __attribute__ ((fallthrough));

  case ';':
  case '{':
  case '}':
  case ',':
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '.':
  case '%':
    token->type = c;
    advance(lexer);
    break;

  case '<':
    if (peek_at(lexer, 1) == '=') {
      token->type = TOKEN_LTE;
      lexer->ptr += 2;
    } else {
      token->type = c;
      lexer->ptr++;
    }
    break;

  case '>':
    if (peek_at(lexer, 1) == '=') {
      token->type = TOKEN_GTE;
      lexer->ptr += 2;
    } else {
      token->type = c;
      lexer->ptr++;
    }
    break;

  case '\n':
    token->type = TOKEN_NEWLINE;
    advance(lexer);
    break;

  case '"':
    token->type = TOKEN_STRING;
    if (lexer_read_string(lexer, &token->value_string)) {
      return 1;
    }
    break;

  case ':':
    if (peek_at(lexer, 1) == '=') {
      lexer->ptr += 2;
      token->type = TOKEN_DEFINE;
      break;
    }

    __attribute__ ((fallthrough));

  default:
    lexer_error(lexer, "Unexpected character '%c'", c);
    lexer->ptr++;
    return 1;
  }

  return 0;
}
