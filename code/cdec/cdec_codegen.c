/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

function void generate_expression(ast_expression *expression)
{
   switch(expression->type)
   {
      case ASTEXPRESSION_LITERAL_INTEGER:
      {
         printf("  mov eax %lld\n", expression->literal_integer.value);
      } break;

      case ASTEXPRESSION_LITERAL_STRING:
      case ASTEXPRESSION_OPERATION_UNARY:
      case ASTEXPRESSION_OPERATION_BINARY:
      {
         printf("  ;; UNHANDLED EXPRESSION TYPE\n");
      } break;
   }
}

function void generate_statement(ast_statement *statement)
{
   switch(statement->type)
   {
      case ASTSTATEMENT_RETURN:
      {
         generate_expression(statement->result);
         printf("  ret\n");
      } break;

      default:
      {
         printf("  ;; UNHANDLED STATEMENT TYPE %d", statement->type);
      } break;
   }
}

function void generate_function(ast_function *func)
{
   printf(".global %s\n", func->name.data);
   printf("%s:\n", func->name.data);

   generate_statement(func->body);

   printf("\n");
}

function void generate_program(ast_program *program)
{
   printf(";;; GENERATED ASSEMBLY CODE\n");

   ast_function *func = program->functions;
   while(func)
   {
      generate_function(program->functions);
      func = program->functions->next;
   }
}
