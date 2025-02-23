/* (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */

#include "SDL3/SDL.h"
#include "desktop.h"

typedef struct {
   SDL_Window *window;
   SDL_Renderer *renderer;
   SDL_Texture *texture;

   int width;
   int height;
   int refresh_rate;
   float seconds_per_frame;

   u64 frame_counter_frequency;
   u64 frame_start_counter;

   bool is_fullscreen;
} sdl_context;

static void sdl_initialize(sdl_context *sdl)
{
   SDL_Init(SDL_INIT_VIDEO);
   SDL_HideCursor();

   sdl->width = 800;
   sdl->height = 600;

   int display_count;
   SDL_DisplayID *displays = SDL_GetDisplays(&display_count);
   if(display_count > 0)
   {
      const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(displays[0]);
      if(mode && mode->refresh_rate != 0)
      {
         sdl->refresh_rate = mode->refresh_rate;
         sdl->width  = mode->w / 2;
         sdl->height = mode->h / 2;
      }
      else
      {
         SDL_Log("Warning: failed to get current display.");
      }
   }
   else
   {
      SDL_Log("Warning: failed to get displays.");
   }

   SDL_CreateWindowAndRenderer("exo desktop", sdl->width, sdl->height, 0, &sdl->window, &sdl->renderer);
   SDL_Log("Desktop Resolution: %dx%d\n", sdl->width, sdl->height);

   SDL_SetRenderVSync(sdl->renderer, 1);
   if(!SDL_SetRenderLogicalPresentation(sdl->renderer, sdl->width, sdl->height, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE))
   {
      SDL_Log("Warning: Failed to set render logical presentation.");
   }

   sdl->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, sdl->width, sdl->height);
   sdl->refresh_rate = 60;

   SDL_Log("Target refresh rate: %d\n", sdl->refresh_rate);
   sdl->seconds_per_frame = 1.0f / sdl->refresh_rate;

   sdl->frame_counter_frequency = SDL_GetPerformanceFrequency();
   sdl->frame_start_counter = SDL_GetPerformanceCounter();
}

static bool sdl_frame_begin(sdl_context *sdl, desktop_input *input, texture backbuffer)
{
   bool keep_running = true;

   for(u32 button_index = 0; button_index < MOUSE_BUTTON_COUNT; ++button_index)
   {
      input->mouse_buttons[button_index].changed_state = false;
   }

   SDL_Event event;
   while(SDL_PollEvent(&event))
   {
      switch(event.type)
      {
         case SDL_EVENT_QUIT:
         {
            keep_running = false;
         } break;

         case SDL_EVENT_MOUSE_BUTTON_DOWN:
         case SDL_EVENT_MOUSE_BUTTON_UP:
         {
            input_state *mouse_button = 0;
            switch(event.button.button)
            {
               case SDL_BUTTON_LEFT:   {mouse_button = input->mouse_buttons + MOUSE_BUTTON_LEFT;} break;
               case SDL_BUTTON_MIDDLE: {mouse_button = input->mouse_buttons + MOUSE_BUTTON_MIDDLE;} break;
               case SDL_BUTTON_RIGHT:  {mouse_button = input->mouse_buttons + MOUSE_BUTTON_RIGHT;} break;
               case SDL_BUTTON_X1:     {mouse_button = input->mouse_buttons + MOUSE_BUTTON_X1;} break;
               case SDL_BUTTON_X2:     {mouse_button = input->mouse_buttons + MOUSE_BUTTON_X2;} break;
            }
            assert(mouse_button);

            mouse_button->is_pressed = event.button.down;
         } break;

         case SDL_EVENT_KEY_DOWN:
         case SDL_EVENT_KEY_UP:
         {
            bool pressed = event.key.down;
            bool repeated = event.key.repeat;
            bool is_alt_pressed = (event.key.mod & SDL_KMOD_ALT);

            SDL_Keycode code = event.key.key;
            if(pressed && !repeated)
            {
               if(code == SDLK_ESCAPE || (is_alt_pressed && code == SDLK_F4))
               {
                  keep_running = false;
               }
               else if(is_alt_pressed && code == SDLK_RETURN)
               {
                  SDL_SetWindowFullscreen(sdl->window, (sdl->is_fullscreen) ? 0 : SDL_WINDOW_FULLSCREEN);
                  sdl->is_fullscreen = !sdl->is_fullscreen;
               }
            }
         } break;
      }
   }

   float mousex, mousey;
   SDL_GetMouseState(&mousex, &mousey);

   int window_width, window_height;
   SDL_GetWindowSize(sdl->window, &window_width, &window_height);

   input->mousex = (s32)((float)mousex * ((float)backbuffer.width / (float)window_width));
   input->mousey = (s32)((float)mousey * ((float)backbuffer.height / (float)window_height));

   if(input->mousex < 0) input->mousex = 0;
   if(input->mousey < 0) input->mousey = 0;
   if(input->mousex >= backbuffer.width)  input->mousex = backbuffer.width;
   if(input->mousey >= backbuffer.height) input->mousey = backbuffer.height;

   return(keep_running);
}

static void sdl_render(sdl_context *sdl, texture backbuffer)
{
   SDL_SetRenderDrawColor(sdl->renderer, 0x18, 0x18, 0x18, 0xFF);
   SDL_RenderClear(sdl->renderer);

   int pitch = backbuffer.width * sizeof(*backbuffer.memory);
   SDL_UpdateTexture(sdl->texture, 0, backbuffer.memory, pitch);
   SDL_RenderTexture(sdl->renderer, sdl->texture, 0, 0);

   SDL_RenderPresent(sdl->renderer);
}

#define SDL_GET_SECONDS_ELAPSED(start, end) ((float)((end) - (start)) / (float)(sdl->frame_counter_frequency))

static void sdl_frame_end(sdl_context *sdl, desktop_input *input)
{
   input->previous_mousex = input->mousex;
   input->previous_mousey = input->mousey;

   u32 sleep_ms = 0;
   u64 frame_end_counter = SDL_GetPerformanceCounter();
   float frame_seconds_elapsed = SDL_GET_SECONDS_ELAPSED(sdl->frame_start_counter, frame_end_counter);
   if(frame_seconds_elapsed < sdl->seconds_per_frame)
   {
      sleep_ms = (u32)(((sdl->seconds_per_frame - frame_seconds_elapsed) * 1000.0f) - 1);
      SDL_Delay(sleep_ms);
   }
   while(frame_seconds_elapsed < sdl->seconds_per_frame)
   {
      frame_end_counter = SDL_GetPerformanceCounter();
      frame_seconds_elapsed = SDL_GET_SECONDS_ELAPSED(sdl->frame_start_counter, frame_end_counter);
   }

   input->frame_count++;
   input->frame_seconds_elapsed = frame_seconds_elapsed;
   input->target_seconds_per_frame = sdl->seconds_per_frame;
   input->sleep_ms = sleep_ms / 1000.0f;

   sdl->frame_start_counter = frame_end_counter;
}

int main(int argument_count, char **arguments)
{
   sdl_context sdl = {0};
   sdl_initialize(&sdl);

   desktop_context desktop = {0};
   desktop_initialize(&desktop, sdl.width, sdl.height);

   while(sdl_frame_begin(&sdl, &desktop.input, desktop.backbuffer))
   {
      desktop_update(&desktop);

      sdl_render(&sdl, desktop.backbuffer);
      sdl_frame_end(&sdl, &desktop.input);
   }

   return(0);
}
