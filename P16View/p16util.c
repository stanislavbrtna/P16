#include "p16util.h"

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
