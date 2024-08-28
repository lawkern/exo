/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <windows.h>
#include <stdio.h>

#include <cdec.h>
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

   OutputDebugStringA(message);
#else
   (void)format;
#endif
}

PLATFORM_ALLOCATE(platform_allocate)
{
   void *result = VirtualAlloc(0, s, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
   return(result);
}

PLATFORM_LOAD_FILE(platform_load_file)
{
   s8 result = s8("");

   WIN32_FIND_DATAA file_data;
   HANDLE find_file = FindFirstFileA(path, &file_data);
   if(find_file == INVALID_HANDLE_VALUE)
   {
      platform_log("WARNING: Failed to find file \"%s\".\n", path);
      return(result);
   }
   FindClose(find_file);


   // NOTE: Store the previous arena location in case we need to back out.
   arena_marker marker = arena_marker_set(a);

   size_t length = (file_data.nFileSizeHigh * (MAXDWORD + 1)) + file_data.nFileSizeLow;
   result.length = length;
   result.data = arena_allocate(a, u8, length + 1);
   if(!result.data)
   {
      platform_log("ERROR: Failed to allocate memory for file \"%s\".\n", path);
      arena_marker_restore(marker);

      return(result);
   }

   // NOTE: ReadFile is limited to reading 32-bit file sizes. As a result, the
   // Win32 platform can't actually use the full 64-bit size_t file size defined
   // in the non-platform code - it caps out at 4GB.

   HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
   DWORD bytes_read;
   if(ReadFile(file, result.data, (DWORD)length, &bytes_read, 0) && length == (size_t)bytes_read)
   {
      result.length = length;
      result.data[result.length] = 0;
   }
   else
   {
      platform_log("ERROR: Failed to read file \"%s.\"\n", path);
      arena_marker_restore(marker);
   }
   CloseHandle(file);

   return(result);
}

PLATFORM_SAVE_FILE(platform_save_file)
{
   bool result = false;

   HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
   if(file != INVALID_HANDLE_VALUE)
   {
      DWORD bytes_written;
      BOOL success = WriteFile(file, memory, (DWORD)s, &bytes_written, 0);

      result = (success && (s == (size)bytes_written));
      if(!result)
      {
         platform_log("ERROR: Failed to write file \"%s.\"\n", path);
      }

      CloseHandle(file);
   }

   return(result);
}
