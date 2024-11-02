/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "desktop.c"

#define SDL_GET_SECONDS_ELAPSED(start, end) ((float)((end) - (start)) / (float)(frame_counter_frequency))

int main(int argument_count, char **arguments)
{
   SDL_Init(SDL_INIT_VIDEO);

   SDL_ShowCursor(SDL_DISABLE);

   texture backbuffer = {DESKTOP_SCREEN_RESOLUTION_X, DESKTOP_SCREEN_RESOLUTION_Y};
   backbuffer.memory = (u32 *)calloc(1, backbuffer.width * backbuffer.height * sizeof(u32));

   SDL_Window *window = SDL_CreateWindow("exo desktop", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, backbuffer.width, backbuffer.height, 0); // SDL_WINDOW_RESIZABLE
   SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
   SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, backbuffer.width, backbuffer.height);

   SDL_RenderSetLogicalSize(renderer, backbuffer.width, backbuffer.height);
   printf("Resolution: %dx%d\n", backbuffer.width, backbuffer.height);

   int target_refresh_rate = 60;
   SDL_DisplayMode mode;
   if(SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) == 0 && mode.refresh_rate != 0)
   {
      target_refresh_rate = mode.refresh_rate;
   }
   printf("Target refresh rate: %d\n", target_refresh_rate);
   float target_seconds_per_frame = 1.0f / target_refresh_rate;

   desktop_input input = {0};

   desktop_storage storage = {0};
   storage.size = MEGABYTES(512);
   storage.memory = (u8 *)calloc(1, storage.size);

   u64 frame_count = 0;
   u64 frame_counter_frequency = SDL_GetPerformanceFrequency();
   u64 frame_start_counter = SDL_GetPerformanceCounter();

   bool is_fullscreen = false;
   bool is_running = true;
   while(is_running)
   {
      for(u32 button_index = 0; button_index < MOUSE_BUTTON_COUNT; ++button_index)
      {
         input.mouse_buttons[button_index].changed_state = false;
      }

      SDL_Event event;
      while(SDL_PollEvent(&event))
      {
         switch(event.type)
         {
            case SDL_QUIT:
            {
               is_running = false;
            } break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
               input_state *mouse_button = 0;
               switch(event.button.button)
               {
                  case SDL_BUTTON_LEFT:   {mouse_button = input.mouse_buttons + MOUSE_BUTTON_LEFT;} break;
                  case SDL_BUTTON_MIDDLE: {mouse_button = input.mouse_buttons + MOUSE_BUTTON_MIDDLE;} break;
                  case SDL_BUTTON_RIGHT:  {mouse_button = input.mouse_buttons + MOUSE_BUTTON_RIGHT;} break;
                  case SDL_BUTTON_X1:     {mouse_button = input.mouse_buttons + MOUSE_BUTTON_X1;} break;
                  case SDL_BUTTON_X2:     {mouse_button = input.mouse_buttons + MOUSE_BUTTON_X2;} break;
               }

               assert(mouse_button);
               mouse_button->changed_state = true;
               if(event.button.state == SDL_PRESSED)
               {
                  mouse_button->is_pressed = true;
               }
               else
               {
                  assert(event.button.state == SDL_RELEASED);
                  mouse_button->is_pressed = false;
               }
            } break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
               bool is_alt_pressed = (event.key.keysym.mod & KMOD_ALT);

               SDL_Keycode code = event.key.keysym.sym;
               if(event.key.state == SDL_PRESSED)
               {
                  if(code == SDLK_ESCAPE || (is_alt_pressed && code == SDLK_F4))
                  {
                     is_running = false;
                  }
                  else if(is_alt_pressed && code == SDLK_RETURN)
                  {
                     SDL_SetWindowFullscreen(window, (is_fullscreen) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                     is_fullscreen = !is_fullscreen;
                  }
               }
            } break;
         }
      }

      int mousex, mousey, window_width, window_height;
      SDL_GetMouseState(&mousex, &mousey);
      SDL_GetWindowSize(window, &window_width, &window_height);

      input.mousex = (s32)((float)mousex * ((float)backbuffer.width / (float)window_width));
      input.mousey = (s32)((float)mousey * ((float)backbuffer.height / (float)window_height));

      if(input.mousex < 0) input.mousex = 0;
      if(input.mousey < 0) input.mousey = 0;
      if(input.mousex >= backbuffer.width)  input.mousex = backbuffer.width;
      if(input.mousey >= backbuffer.height) input.mousey = backbuffer.height;

      update(&backbuffer, &input, &storage);

      input.previous_mousex = input.mousex;
      input.previous_mousey = input.mousey;

      SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
      SDL_RenderClear(renderer);
      SDL_UpdateTexture(texture, 0, backbuffer.memory, backbuffer.width * sizeof(u32));
      SDL_RenderCopy(renderer, texture, 0, 0);
      SDL_RenderPresent(renderer);

      u32 sleep_ms = 0;
      u64 frame_end_counter = SDL_GetPerformanceCounter();
      float frame_seconds_elapsed = SDL_GET_SECONDS_ELAPSED(frame_start_counter, frame_end_counter);
      if(frame_seconds_elapsed < target_seconds_per_frame)
      {
         sleep_ms = (u32)(((target_seconds_per_frame - frame_seconds_elapsed) * 1000.0f) - 1);
         SDL_Delay(sleep_ms);
      }
      while(frame_seconds_elapsed < target_seconds_per_frame)
      {
         frame_end_counter = SDL_GetPerformanceCounter();
         frame_seconds_elapsed = SDL_GET_SECONDS_ELAPSED(frame_start_counter, frame_end_counter);
      }
      if((frame_count++ % target_refresh_rate) == 0)
      {
         float frame_ms = frame_seconds_elapsed * 1000.0f;
         float target_ms = target_seconds_per_frame * 1000.0f;
         float frame_utilization = ((frame_ms - sleep_ms) / target_ms * 100.0f);

         printf("frame:%0.03fms ", frame_ms);
         printf("target:%0.03fms, ", target_ms);
         printf("sleep:%ums, ", sleep_ms);
         printf("work:%.2f%%\n", frame_utilization);
      }

      input.frame_count++;
      input.frame_seconds_elapsed = frame_seconds_elapsed;
      input.target_seconds_per_frame = target_seconds_per_frame;

      frame_start_counter = frame_end_counter;
   }

   return(0);
}
