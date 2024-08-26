#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define KEYWORDS_NAMES                          \
   X(begin)                                     \
   X(import)                                    \
   X(function)                                  \
   X(var)                                       \
   X(struct)                                    \
   X(union)                                     \
   X(enum)                                      \
   X(return)                                    \
   X(if)                                        \
   X(else)                                      \
   X(for)                                       \
   X(range)                                     \
   X(switch)                                    \
   X(break)                                     \
   X(continue)                                  \
   X(defer)                                     \
   X(int)                                       \
   X(string)                                    \
   X(end)

#define intern_stringz(s) intern_string_length((s), lengthof(s))

typedef struct {
   size index;
   size count;
   char *characters;
} text_stream;

typedef enum {
   TOKENTYPE_ENDOFSTREAM,

   TOKENTYPE_OPENPAREN,
   TOKENTYPE_CLOSEPAREN,
   TOKENTYPE_OPENBRACE,
   TOKENTYPE_CLOSEBRACE,
   TOKENTYPE_OPENBRACKET,
   TOKENTYPE_CLOSEBRACKET,
   TOKENTYPE_COLON,
   TOKENTYPE_SEMICOLON,
   TOKENTYPE_COMMA,
   TOKENTYPE_PERIOD,

   TOKENTYPE_BITWISE_NOT,
   TOKENTYPE_LOGICAL_NOT,

   TOKENTYPE_NAME,
   TOKENTYPE_KEYWORD,
   TOKENTYPE_INTEGER,
   TOKENTYPE_STRING,

   // NOTE: Binary operators
   TOKENTYPE_MUL,
   TOKENTYPE_DIV,
   TOKENTYPE_MOD,
   TOKENTYPE_SHIFT_LEFT,
   TOKENTYPE_SHIFT_RIGHT,
   TOKENTYPE_BITWISE_AND,

   TOKENTYPE_ADD,
   TOKENTYPE_SUB,
   TOKENTYPE_BITWISE_OR,
   TOKENTYPE_BITWISE_XOR,

   TOKENTYPE_EQ,
   TOKENTYPE_NE,
   TOKENTYPE_GT,
   TOKENTYPE_GTE,
   TOKENTYPE_LT,
   TOKENTYPE_LTE,

   TOKENTYPE_LOGICAL_AND,

   TOKENTYPE_LOGICAL_OR,


   // NOTE: Assignment operators
   TOKENTYPE_ASSIGN,
   TOKENTYPE_ASSIGN_COLON,
   TOKENTYPE_ASSIGN_ADD,
   TOKENTYPE_ASSIGN_SUB,
   TOKENTYPE_ASSIGN_MUL,
   TOKENTYPE_ASSIGN_DIV,
   TOKENTYPE_ASSIGN_MOD,
   TOKENTYPE_ASSIGN_AND,
   TOKENTYPE_ASSIGN_OR,
   TOKENTYPE_ASSIGN_XOR,
   TOKENTYPE_ASSIGN_NOT,
   TOKENTYPE_ASSIGN_SHIFT_LEFT,
   TOKENTYPE_ASSIGN_SHIFT_RIGHT,

   TOKENTYPE_INCREMENT,
   TOKENTYPE_DECREMENT,

   TOKENTYPE_COUNT,
} tokentype;

global char *tokentype_names[TOKENTYPE_COUNT] = {
   [TOKENTYPE_ENDOFSTREAM] = "EOS",

   [TOKENTYPE_OPENPAREN]    = "(",
   [TOKENTYPE_CLOSEPAREN]   = ")",
   [TOKENTYPE_OPENBRACE]    = "{",
   [TOKENTYPE_CLOSEBRACE]   = "}",
   [TOKENTYPE_OPENBRACKET]  = "[",
   [TOKENTYPE_CLOSEBRACKET] = "]",
   [TOKENTYPE_COLON]        = ":",
   [TOKENTYPE_SEMICOLON]    = ";",
   [TOKENTYPE_COMMA]        = ",",
   [TOKENTYPE_PERIOD]       = ".",
   [TOKENTYPE_BITWISE_NOT]  = "~",
   [TOKENTYPE_LOGICAL_NOT]  = "!",

   [TOKENTYPE_NAME]    = "name",
   [TOKENTYPE_KEYWORD] = "keyword",
   [TOKENTYPE_INTEGER] = "int",
   [TOKENTYPE_STRING]  = "string",

   [TOKENTYPE_MUL]         = "*",
   [TOKENTYPE_DIV]         = "/",
   [TOKENTYPE_MOD]         = "%",
   [TOKENTYPE_SHIFT_LEFT]  = "<<",
   [TOKENTYPE_SHIFT_RIGHT] = ">>",
   [TOKENTYPE_BITWISE_AND] = "&",
   [TOKENTYPE_ADD]         = "+",
   [TOKENTYPE_SUB]         = "-",
   [TOKENTYPE_BITWISE_OR]  = "|",
   [TOKENTYPE_BITWISE_XOR] = "^",
   [TOKENTYPE_EQ]          = "==",
   [TOKENTYPE_NE]          = "!=",
   [TOKENTYPE_GT]          = ">",
   [TOKENTYPE_GTE]         = ">=",
   [TOKENTYPE_LT]          = "<",
   [TOKENTYPE_LTE]         = "<=",
   [TOKENTYPE_LOGICAL_AND] = "&&",
   [TOKENTYPE_LOGICAL_OR]  = "||",


   [TOKENTYPE_ASSIGN]             = "=",
   [TOKENTYPE_ASSIGN_COLON]       = ":=",
   [TOKENTYPE_ASSIGN_ADD]         = "+=",
   [TOKENTYPE_ASSIGN_SUB]         = "-=",
   [TOKENTYPE_ASSIGN_MUL]         = "*=",
   [TOKENTYPE_ASSIGN_DIV]         = "/=",
   [TOKENTYPE_ASSIGN_MOD]         = "%=",
   [TOKENTYPE_ASSIGN_AND]         = "&=",
   [TOKENTYPE_ASSIGN_OR]          = "|=",
   [TOKENTYPE_ASSIGN_XOR]         = "^=",
   [TOKENTYPE_ASSIGN_NOT]         = "~=",
   [TOKENTYPE_ASSIGN_SHIFT_LEFT]  = "<<=",
   [TOKENTYPE_ASSIGN_SHIFT_RIGHT] = ">>=",

   [TOKENTYPE_INCREMENT] = "++",
   [TOKENTYPE_DECREMENT] = "--",
};


typedef struct {
   tokentype type;
   union
   {
      char *name;
      u64 value_integer;
      char *value_string;
   };
} lexical_token;

typedef struct {
   size index;
   size count;
   lexical_token tokens[4096];
} token_stream;

typedef struct {
   size index;
   size count;
   s8 strings[4096];
} string_table;
