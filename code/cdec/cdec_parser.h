#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

typedef enum {
   ASTEXPRESSION_LITERAL_INTEGER,
   ASTEXPRESSION_LITERAL_STRING,
   ASTEXPRESSION_OPERATION_UNARY,
   ASTEXPRESSION_OPERATION_BINARY,
   ASTEXPRESSION_FUNCTIONCALL,
} ast_expression_type;

typedef struct {
   s8 name;
} ast_identifier;

typedef struct {
   u64 value;
} ast_literal_integer;

typedef struct {
   s8 value;
} ast_literal_string;

typedef struct ast_expression {
   ast_expression_type type;
   union
   {
      ast_literal_integer literal_integer;
      ast_literal_string literal_string;
      struct // Binary operation
      {
         tokentype operator;
         struct ast_expression *expression;
         struct ast_expression *expression2;
      };
      struct // Function call
      {
         ast_identifier *name;
         struct ast_expression *arguments;
      };
   };
   struct ast_expression *next;
} ast_expression;

typedef enum {
   ASTSTATEMENT_RETURN,
   ASTSTATEMENT_IF,
   ASTSTATEMENT_FOR,
   ASTSTATEMENT_VAR,
   ASTSTATEMENT_IMPORT,
   ASTSTATEMENT_EXPRESSION,
} ast_statement_type;

typedef struct ast_statement {
   ast_statement_type type;

   // return/expression
   ast_expression *result;

   // if/for
   ast_expression *condition;
   struct ast_statement *body;

   // declaration
   ast_identifier *identifier;
   ast_identifier *typename;

   struct ast_statement *next;
} ast_statement;

typedef struct ast_parameter {
   ast_identifier *name;
   ast_identifier *type;
   struct ast_parameter *next;
} ast_parameter;

typedef struct ast_function {
   s8 name;
   ast_parameter *parameters;
   ast_identifier *return_type;
   ast_statement *body;

   struct ast_function *next;
} ast_function;

typedef struct {
   ast_statement *imports;
   ast_function *functions;
} ast_program;
