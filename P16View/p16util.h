
#ifndef P16_UTILS_H
#define P16_UTILS_H

#include "p16view.h"
#include "p16util.h"
#include "SDA_fs/sda_fs_pc.h"

typedef struct {
  uint8_t version;
  uint16_t imageWidth;
  uint16_t imageHeight;
  uint8_t  storageMode;
  uint8_t  dataOffset;
} p16Header;

typedef struct {
  uint32_t init;
  uint32_t fpos;
  uint16_t prevVal;
  uint16_t repeat;
} p16State;

uint16_t p16_get_pixel(svp_file * fp, p16Header * header, p16State * state);
uint8_t p16_get_header(svp_file * fp, p16Header * header);

#endif