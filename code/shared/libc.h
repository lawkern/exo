#pragma once

/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

void *memmove(void *destination, const void *source, size_t size)
{
   unsigned char *destination_bytes = (unsigned char *)destination;
   unsigned char *source_bytes = (unsigned char *)source;

   if(destination_bytes < source_bytes)
   {
      for(size_t index = 0; index < size; index++)
      {
         destination_bytes[index] = source_bytes[index];
      }
   }
   else
   {
      for(size_t index = size; index != 0; index--)
      {
         destination_bytes[index - 1] = source_bytes[index - 1];
      }
   }
   return(destination);
}

int memcmp(const void *a, const void *b, size_t size)
{
   unsigned char *a_bytes = (unsigned char *)a;
   unsigned char *b_bytes = (unsigned char *)b;

   for(size_t index = 0; index < size; index++)
   {
      if(a_bytes[index] < b_bytes[index])
      {
         return(-1);
      }
      else if(b_bytes[index] < a_bytes[index])
      {
         return(1);
      }
   }
   return(0);
}

void *memset(void *destination, int value, size_t size)
{
   unsigned char *destination_bytes = (unsigned char *)destination;

   for(size_t index = 0; index < size; index++)
   {
      destination_bytes[index] = (unsigned char)value;
   }
   return(destination);
}

void *memcpy(void * restrict destination, const void * restrict source, size_t size)
{
   unsigned char *destination_bytes = (unsigned char *)destination;
   unsigned char *source_bytes = (unsigned char *)source;

   for(size_t index = 0; index < size; index++)
   {
      destination_bytes[index] = source_bytes[index];
   }
   return(destination);

}

size_t strlen(const char* string)
{
   size_t result = 0;
   while(string[result])
   {
      result++;
   }
   return(result);
}

__attribute__((__noreturn__)) void abort(void)
{
   asm volatile("hlt");
   while(1)
   {
   }
   __builtin_unreachable();
}
