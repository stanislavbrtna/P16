#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <SDL2/SDL.h>

#include "p16util.h"
#include "SDA_fs/sda_fs_pc.h"

// sdl window dimensions
const int SCREEN_WIDTH = 455;
const int SCREEN_HEIGHT = 733;

#define FB_W 1024
#define FB_H 1024

SDL_Renderer* gRenderer;
SDL_Texture * gTexture;

SDL_Texture * bgTexture;

uint8_t quit;

// software framebuffer
uint16_t sw_fb[FB_W][FB_H];


uint8_t p16_get_header(svp_file * fp, p16Header * header) {
  svp_fseek(fp, 0);

  header->version = 0;
  header->imageWidth = 0;
  header->imageHeight = 0;
  header->storageMode = 0;
  header->dataOffset = 0;

  if (svp_fread_u8(fp) != 0x1b) {
    return 1;
  }

  if (svp_fread_u8(fp) != 'P') {
    return 1;
  }

  if (svp_fread_u8(fp) != 'S') {
    return 1;
  }

  if (svp_fread_u8(fp) != 'M') {
    return 1;
  }

  if (svp_fread_u8(fp) != '1') {
    return 1;
  }

  if (svp_fread_u8(fp) != '6') {
    return 1;
  }

  header->version = svp_fread_u8(fp);

  if(header->version == 1) {
    header->imageWidth |= svp_fread_u8(fp);
    header->imageWidth |= svp_fread_u8(fp) << 8;

    header->imageHeight |= svp_fread_u8(fp);
    header->imageHeight |= svp_fread_u8(fp) << 8;

    header->storageMode = svp_fread_u8(fp);
    header->dataOffset = svp_fread_u8(fp);

    // convinient seek to data start
    if (header->dataOffset != 1) {
      svp_fseek(fp, svp_ftell(fp) + header->dataOffset - 1);
    }
  } else {
    printf("Can parse p16 version 1 only\n");
    return 1;
  }
  return 0;
}

uint16_t p16_get_pixel(svp_file * fp, p16Header * header, p16State * state) {
  uint16_t color;

  if (header->storageMode == 0) {
    svp_fread(fp, &color, sizeof(color));
    return color;
  }
  if (header->storageMode == 1) {
    if (state->init == 0) {
      svp_fread(fp, &color, sizeof(color));
      state->fpos = svp_ftell(fp);
      state->init = 1;
      state->prevVal = 0;
      state->repeat = 0;

      state->prevVal = color;
      return color;
    } else {
      if (state->repeat == 0) {
        svp_fread(fp, &color, sizeof(color));
        if (state->prevVal == color) {
          svp_fread(fp, &state->repeat, sizeof(state->repeat));
          state->repeat--;
          state->prevVal = color;
          return color;
        } else {
          state->prevVal = color;
          return color;
        }
      } else {
        state->repeat--;
        return state->prevVal;
      }
    }
  }

  return 0;
}


// clears software framebuffer
void fb_clear() {
  int a,b;
  for (a = 0; a < FB_W; a++) {
    for (b = 0; b < FB_H; b++) {
      sw_fb[a][b] = 0xFFFF;
    }
  }
}

// renders swfb

void fb_copy_to_renderer() {
  int a,i;
  SDL_Rect dstrect;
  uint8_t r, g, b;
  Uint32* pixels = 0;
  int pitch = 0;
  int format;

  SDL_LockTexture(gTexture, 0, (void**)&pixels, &pitch);

  for (a = 0; a < FB_W; a++) {
    for (i = 0; i < FB_H; i++) {
      r = (uint8_t)(((float)((sw_fb[a][i]>>11)&0x1F)/32)*256);
      g = (uint8_t)(((float)(((sw_fb[a][i]&0x07E0)>>5)&0x3F)/64)*256);
      b = (uint8_t)(((float)(sw_fb[a][i]&0x1F)/32)*256);

      pixels[i * FB_W + a] = r << 24 | g << 16 | b << 8 | SDL_ALPHA_OPAQUE;
    }
  }

  SDL_UnlockTexture(gTexture);

  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = FB_W;
  dstrect.h = FB_H;

  SDL_RenderCopy(gRenderer, gTexture, NULL, &dstrect);
}


void sda_sim_loop() {
  static time_t ti;
  static struct tm * timeinfo;

  SDL_Event e;

  while(SDL_PollEvent(&e) != 0) {
    if(e.type == SDL_QUIT) {
      quit = 1;
    }
  }

  SDL_RenderClear(gRenderer);
  fb_copy_to_renderer();
  SDL_RenderPresent(gRenderer);
}


int main(int argc, char *argv[]) {
  //events

  SDL_Window* window = NULL;
  //time
  quit = 0;
  uint32_t sdl_time;

  printf("P16 View 1.2\n");
  svp_mount();

  // usefull vars
  p16Header header;
  p16State imageState;
  svp_file f;
  // read the header

  if (svp_fopen_read(&f, argv[1]) == 0) {
    printf("file %s does not exist!\n", argv[1]);
    return 0;
  }

  if(p16_get_header(&f, &header)){
    return 0;
  }

  printf("w: %u h:%u ver: %u fmt:%u\n", header.imageWidth, header.imageHeight, header.version, header.storageMode);

  if (header.imageWidth > FB_W || header.imageHeight > FB_H) {
    printf("Image is too big to display\n");
    return 0;
  }

  SDL_Init( SDL_INIT_VIDEO );
  if (header.imageWidth > 100) {
    SDL_CreateWindowAndRenderer(header.imageWidth, header.imageHeight, 0, &window, &gRenderer);
  } else {
    SDL_CreateWindowAndRenderer(256, header.imageHeight + 20, 0, &window, &gRenderer);    
  }

  fb_clear();

  imageState.init = 0;

  for (int a = 0; a < header.imageHeight; a++) {
    for (int i = 0; i < header.imageWidth; i++) {
      sw_fb[i][a] = p16_get_pixel(&f, &header, &imageState);
    }
  }

  SDL_SetWindowTitle(window, argv[1]);

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
  SDL_RenderClear(gRenderer);

  gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);



  while (!quit) { // quit is handled only from SDL_QUIT
    sdl_time = SDL_GetTicks();
    sda_sim_loop();

    if (SDL_GetTicks() - sdl_time < 33) {
      SDL_Delay(33 - (SDL_GetTicks() - sdl_time));
    }
  }

  // Destroy window
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(window); // Quit SDL subsystems
  window = NULL;
  gRenderer = NULL;
  SDL_Quit();
  return 0;
}
