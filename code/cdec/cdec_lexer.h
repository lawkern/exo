#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define KEYWORDS_NAMES                          \
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
   X(string)

#define intern_stringz(s) intern_string(s8(s))

typedef enum {
   // NOTE: tokentype uses ASCII character literals as their own tokentype
   // identifiers. The first custom tokentype begins at 128 as a result.

   TOKENTYPE_IDENTIFIER = 128,
   TOKENTYPE_INTEGER,
   TOKENTYPE_STRING,

   // NOTE: Assignment operators
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

   // NOTE: Comparison operators
   TOKENTYPE_EQ,
   TOKENTYPE_NE,
   TOKENTYPE_GTE,
   TOKENTYPE_LTE,
   TOKENTYPE_AND,
   TOKENTYPE_OR,

   TOKENTYPE_INCREMENT,
   TOKENTYPE_DECREMENT,
   TOKENTYPE_SHIFT_LEFT,
   TOKENTYPE_SHIFT_RIGHT,

   TOKENTYPE_COUNT,
} tokentype;

typedef struct {
   tokentype type;
   union
   {
      u8 *name;
      u64 value_integer;
      u8 *value_string;
   };
} token;

typedef struct {
   u32 index;
   u32 count;
   token tokens[2048];
} token_stream;

typedef struct {
   u32 count;
   s8 strings[4096];
} string_intern_table;
