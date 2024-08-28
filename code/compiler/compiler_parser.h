#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

typedef struct {
   char *name;
   b32 is_pointer;
} ast_typespec;

typedef enum {
   AST_EXPRESSION_LITERAL_INTEGER,
   AST_EXPRESSION_LITERAL_STRING,
   AST_EXPRESSION_NAME,
   AST_EXPRESSION_OPERATION_UNARY,
   AST_EXPRESSION_OPERATION_BINARY,
   AST_EXPRESSION_OPERATION_TERNARY,
   AST_EXPRESSION_FUNCTIONCALL,
} ast_expression_type;

typedef struct ast_expression {
   ast_expression_type type;
   ast_typespec *typespec;
   union
   {
      u64 value_integer;
      char *value_string;

      struct // Unary/Binary operation
      {
         tokentype operator;
         struct ast_expression *expression;
         struct ast_expression *expression2;
      };

      struct // Function call
      {
         char *name;
         struct ast_expression *arguments;
      };
   };
   struct ast_expression *next;
} ast_expression;

typedef enum {
   AST_STATEMENT_RETURN,
   AST_STATEMENT_IF,
   AST_STATEMENT_FOR,
   AST_STATEMENT_VAR,
   AST_STATEMENT_IMPORT,
   AST_STATEMENT_EXPRESSION,
} ast_statement_type;

typedef struct ast_statement {
   ast_statement_type type;

   union
   {
      struct {
         char *name;
      } import_stmt;

      struct {
         ast_expression *expression;
      } return_stmt;

      struct {
         ast_expression *condition;
         struct ast_statement *then_block;
         struct ast_statement *else_block;
      } if_stmt;

      struct {
         ast_expression *condition;
         struct ast_expression *pre;
         struct ast_expression *post;
         struct ast_statement *body;
      } for_stmt;

      struct {
         char *name;
         ast_typespec *typespec;
         ast_expression *expression;
      } var_stmt;
   };
   struct ast_statement *next;
} ast_statement;

typedef struct ast_parameter {
   char *name;
   ast_typespec *typespec;
   struct ast_parameter *next;
} ast_parameter;

typedef struct ast_function {
   char *name;
   ast_parameter *parameters;
   ast_typespec *return_type;
   ast_statement *body;

   struct ast_function *next;
} ast_function;

typedef struct {
   ast_statement *imports;
   ast_function *functions;
} ast_program;
