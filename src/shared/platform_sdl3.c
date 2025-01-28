/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */

#include "SDL3/SDL.h"
#include "platform.h"

PLATFORM_LOG(platform_log)
{
#if DEVELOPMENT_BUILD
   va_list arguments;
   va_start(arguments, format);
   {
      SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, arguments);
   }
   va_end(arguments);
#else
   (void)format;
#endif
}

PLATFORM_ALLOCATE(platform_allocate)
{
   void *result = SDL_calloc(1, s);
   return(result);
}

PLATFORM_LOAD_FILE(platform_load_file)
{
   string8 result = string8("");

   size_t length = 0;
   void *data = SDL_LoadFile(path, &length);

   if(data)
   {
      result.data = data;
      result.length = length;
   }
   else
   {
      platform_log("ERROR: Failed to read file \"%s\".\n", path);
   }

   return(result);
}

PLATFORM_SAVE_FILE(platform_save_file)
{
   bool result = SDL_SaveFile(path, memory, s);
   if(!result)
   {
      platform_log("ERROR: Failed to save file: \"%s\".\n", path);
   }

   return(result);
}
