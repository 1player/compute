#ifndef COMMON_H
#define COMMON_H

enum token_type {
  TOKEN_EOF = 0,
  TOKEN_ID = 256,
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
  unsigned int line;
} lexer_t;

int lexer_create(lexer_t *lexer, char *input);
int lexer_next(lexer_t *lexer, token_t *token);

#endif
