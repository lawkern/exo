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

typedef struct ast_expression {
   ast_expression_type type;
   union
   {
      u64 value_integer;
      u8 *value_string;

      struct // Unary/Binary operation
      {
         tokentype operator;
         struct ast_expression *expression;
         struct ast_expression *expression2;
      };

      struct // Function call
      {
         u8 *name;
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
   u8 *name;
   u8 *type_name;

   struct ast_statement *next;
} ast_statement;

typedef struct ast_parameter {
   u8 *name;
   u8 *type_name;
   struct ast_parameter *next;
} ast_parameter;

typedef struct ast_function {
   u8 *name;
   ast_parameter *parameters;
   u8 *return_type;
   ast_statement *body;

   struct ast_function *next;
} ast_function;

typedef struct {
   ast_statement *imports;
   ast_function *functions;
} ast_program;
