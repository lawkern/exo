/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

PLATFORM_LOG(platform_log)
{
#if DEVELOPMENT_BUILD
   char message[1024];

   va_list arguments;
   va_start(arguments, format);
   {
      vsnprintf(message, sizeof(message), format, arguments);
   }
   va_end(arguments);

   printf("%s", message);
#else
   (void)format;
#endif
}

PLATFORM_ALLOCATE(platform_allocate)
{
   void *result = mmap(0, s, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   return(result);
}

PLATFORM_FREE_FILE(platform_free_file)
{
   if(munmap(file->memory, file->size) != 0)
   {
      platform_log("ERROR: macOS failed to deallocate the file.");
   }

   memset(file, 0, sizeof(*file));
}

PLATFORM_LOAD_FILE(platform_load_file)
{
   platform_file result = {0};

   struct stat file_information;
   if(stat(path, &file_information) == -1)
   {
      platform_log("ERROR (%d): macOS failed to read file size of file: \"%s\".\n", errno, path);
      return(result);
   }

   int file = open(path, O_RDONLY);
   if(file == -1)
   {
      platform_log("ERROR (%d): macOS failed to open file: \"%s\".\n", path, errno);
      return(result);
   }

   size_t size = file_information.st_size;

   result.memory = mmap(0, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   if(result.memory != MAP_FAILED)
   {
      result.size = size;
      read(file, result.memory, result.size);
   }
   else
   {
      platform_log("ERROR: macOS failed to allocate memory for file: \"%s\".\n", path);
   }

   close(file);

   return(result);
}

PLATFORM_SAVE_FILE(platform_save_file)
{
   bool result = false;

   int file = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
   if(file != -1)
   {
      ssize_t bytes_written = write(file, memory, s);
      result = (bytes_written == s);

      if(!result)
      {
         platform_log("ERROR (%d): macOS failed to write file: \"%s\".\n", errno, path);
      }

      close(file);
   }
   else
   {
      platform_log("ERROR (%d): macOS failed to open file: \"%s\".\n", errno, path);
   }

   return(result);
}
