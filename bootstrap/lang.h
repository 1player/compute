#ifndef LANG_H
#define LANG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  void **elements;
  int size;
  int capacity;
} array_t;

void array_init(array_t *ary);
void array_append(array_t *ary, void *el);
void array_resize(array_t *ary, int new_capacity);
void *array_pop_start(array_t *ary);

//

enum token_type {
  TOKEN_EOF = 0,
  TOKEN_NEWLINE = 256,
  TOKEN_ID,
  TOKEN_STRING,
  TOKEN_NUMBER,
};

typedef struct token {
  enum token_type type;

  union {
    char *value_id;
    char *value_string;
    int value_number;
  };
} token_t;

typedef struct lexer {
  char *ptr;
  char *file;
  unsigned int line;
} lexer_t;

int lexer_create(lexer_t *lexer, char *file, char *input);
int lexer_next(lexer_t *lexer, token_t *token);

//

enum literal_type {
  LITERAL_NUMBER,
  LITERAL_STRING,
};

enum expr_type {
  EXPR_LITERAL,
  EXPR_IDENTIFIER,
  EXPR_SELF,
  EXPR_BINARY_SEND,
  EXPR_SEND,
};

typedef struct expr_t {
  enum expr_type type;

  union {
    struct {
      enum literal_type type;
      int value_number;
      char *value_string;
    } literal;

    struct {
      struct expr_t *receiver;
      struct expr_t *selector;
      array_t *args;
    } send;

    struct {
      char *name;
    } identifier;

    struct {
      struct expr_t *left;
      struct expr_t *selector;
      struct expr_t *right;
    } binary_send;
  };
} expr_t;

void expr_dump(expr_t *expr);

//

typedef struct {
  lexer_t lexer;
  token_t cur_token;
  token_t prev_token;
  bool had_errors;
} parser_t;

typedef struct toplevel_t {
  array_t exprs;
} toplevel_t;

toplevel_t *parser_parse_toplevel(parser_t *parser, const char *file, const char *input);
expr_t *parser_parse_expression(parser_t *parser, const char *input);

//

void panic(char *msg, ...);
void info(char *msg, ...);
void error(char *file, int line, char *msg, va_list args);

#endif
