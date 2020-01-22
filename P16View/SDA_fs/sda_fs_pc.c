/*
Copyright (c) 2018 Stanislav Brtna

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include "sda_fs_pc.h"

uint8_t sd_mounted;

/*konvence jest: pokud ok, tak vracíme nulu*/

uint8_t svp_fopen_read(svp_file *fp, uint8_t *fname) {

  if (!sd_mounted) {
    return 0;
  }

  fp->fPointer = fopen(fname, "rb");
  if (fp->fPointer) {
    return 1;
  } else {
    return 0;
  }

}

uint8_t svp_fopen_rw(svp_file *fp, uint8_t *fname) {

  if (!sd_mounted) {
    return 0;
  }

  fp->fPointer = fopen(fname, "ab");
  fclose(fp->fPointer);

  fp->fPointer = fopen(fname, "r+b");
  if (fp->fPointer) {
    return 1;
  } else {
    return 0;
  }
}


uint8_t svp_fexists(uint8_t *fname) {
svp_file fp;

  if (!sd_mounted) {
    return 0;
  }

  fp.fPointer = fopen(fname, "r");
  if (fp.fPointer) {
    //file exists
    fclose(fp.fPointer);
    return 1;
  } else {
    //fail
    return 0;
  }
}

uint8_t svp_fread_u8(svp_file *fp) {
  uint8_t x;

  if(fread(&x, sizeof(x), 1, fp->fPointer) == 0){
    // we perhaps read from empty stream in some parser, the error messages
    // spammed the stdout, they are temporarily disabled
    //printf("fread error!\n");
  }
  return x;
}

uint8_t svp_fread(svp_file *fp, void *target, uint32_t size) {
  if(fread(target, size, 1, fp->fPointer) == 0) {
    //printf("fread error! (2)\n");
  }
  return 0;
}

void svp_fwrite_u8(svp_file *fp, uint8_t val) {
  if(fwrite(&val, sizeof(uint8_t), 1, fp->fPointer) == 0){
    //printf("fwrite error! (1)\n");
  }
}

void svp_fwrite(svp_file *fp, void *target, uint32_t size) {
  if(fwrite(target, size, 1, fp->fPointer) == 0){
    //printf("fwrite error! (2)\n");
  }
}

uint8_t svp_feof(svp_file *fp) {
  if (ftell(fp->fPointer) >= svp_get_size(fp))
    return 1;
  else
    return 0;
}

uint8_t svp_fclose(svp_file *fp) {

  return fclose(fp->fPointer);
}

uint8_t svp_fseek(svp_file *fp, uint32_t offset) {
  fseek(fp->fPointer, offset, SEEK_SET);
  return 0;
}

uint32_t svp_get_size(svp_file *fp) {
  uint32_t siz = 0;
  uint32_t prev;
  prev = ftell(fp->fPointer);
  fseek(fp->fPointer, 0, SEEK_END);
  siz = ftell(fp->fPointer);
  fseek(fp->fPointer, prev, SEEK_SET);
  return siz;
}

uint32_t svp_ftell(svp_file *fp) {
  return ftell(fp->fPointer);
}


void svp_truncate(svp_file *fp) {
  ftruncate(fileno(fp->fPointer), ftell(fp->fPointer));
}

uint8_t svp_rename(uint8_t *source, uint8_t *dest) {
  if (rename(source, dest) == 0) {
    return 1;
  } else {
    return 0;
  }
}


uint8_t exten[8];
static DIR *d;


uint8_t svp_strcmp_ext(uint8_t *s1, uint8_t *s_ext) {
  uint16_t x = 0;
  uint16_t y = 0;
  //printf("comparing: %s a %s : ", s1, s_ext);
  while(s1[x] != '.') {
    if (s1[x] == 0) {
      if (s_ext[0] == 0) {
        return 1; // if there is no extension, we list all the files
      } else {
        return 0;
      }
      //printf("fail! dot not found\n");
    }
    x++;
  }
  x++;

  while (s_ext[y] != 0) {
    if (s1[x+y] != s_ext[y]) {
      //printf("fail!\n");
      return 0;
    }
    y++;
  }
  //printf("pass!\n");
  return 1;
}

/*
uint8_t svp_extFindNext(uint8_t *outStr, uint16_t len) {
  struct dirent *dir;
  while (1) {
    dir = readdir(d);
    //printf("dir: %d\n", dir);
    if(dir) {
      if ((dir->d_name[0] == '.' && dir->d_name[1] == 0)
           || (dir->d_name[0] == '.' && dir->d_name[1] == '.' && dir->d_name[2] == 0)) {
        continue;
      }
      if (svp_strcmp_ext(dir->d_name, exten)) {
        sda_strcp(dir->d_name, outStr, len);
        return 1;
      } else {
        continue;
      }
    } else {
      closedir(d);
      return 0;
    }
  }
}

uint8_t svp_extFind(uint8_t *outStr, uint16_t len, uint8_t *extension, uint8_t *directory){
  sda_strcp(extension, exten, 7);
  d = opendir(directory);
  if (d){
    return svp_extFindNext(outStr, len);
  }
  return 0;
}*/

uint8_t svp_open_dir(svp_dir *dp, uint8_t *path) {
  dp->d = opendir(path);
  if (dp->d) {
    return 0;
  } else {
    return 1;
  }
}

uint8_t svp_close_dir(svp_dir *dp) {
  closedir(dp->d);
  return 0;
}

uint8_t svp_strcmp(uint8_t *a, uint8_t *b) {
  uint16_t x = 0;
  uint8_t retval = 1;
  //printf("comp: %s a %s\n", a, b);
  while (x < 100) {
    if ((a[x] == 0) || (b[x] == 0)) {
      if (a[x] != b[x]) {
        retval = 0;
      }
      break;
    } else {
      if (a[x] != b[x]){
        retval = 0;
      }
      x++;
    }
  }
  return retval;
}

uint16_t svp_strlen(uint8_t *str) {
  uint16_t x = 0;

  while(str[x] != 0) {
    x++;
  }

  return x + 1; //vrátí len i s terminátorem
}

uint8_t svp_chdir(uint8_t* path) {
  if (chdir(path)){
    return 1;
  } else {
    return 0;
  }
}

uint8_t svp_getcwd(uint8_t* buf, uint16_t len) {
  if (getcwd(buf, len)){
    return 1;
  } else {
    return 0;
  }
}

uint8_t svp_unlink(uint8_t* path) {
  unlink(path);
  return 0;
}

uint8_t svp_is_dir(uint8_t* path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != 0) {
    return 0;
  }
  if (S_ISDIR(path_stat.st_mode)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t svp_mkdir(uint8_t* path) {
  if (mkdir(path, 0777)) {
    return 0;
  } else {
    return 1;
  }
}


void svp_fsync(svp_file *fp) {
  printf("Fsync.\n");
}

uint8_t svp_mount() {
  printf("Volume mounted (in simulation)\n");
  sd_mounted = 1;
  return 0;
}

void svp_umount() {
  sd_mounted = 0;

  printf("Volume umounted (in simulation)\n");
}

uint8_t svp_getMounted() {
  return sd_mounted;
}

// because it was mounted before the svp init
void svp_setMounted(uint8_t val) {
  sd_mounted = val;
}

/* File Find example:
uint8_t buffer[32];
uint8_t retval;

printf("extFind: print all C files\n");

retval=svp_extFind(buffer, 30, "c", ".");

while (retval){
  printf("file: %s\n", buffer);
  retval = svp_extFindNext(buffer, 30);
}

*/

