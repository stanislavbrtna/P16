#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

uint8_t svp_fread_u8(FILE *fp) {
  uint8_t x;

  fread(&x, sizeof(x), 1, fp);
  return x;
}

void svp_fwrite_u8(FILE *fp, uint8_t val) {
  if(fwrite(&val, sizeof(uint8_t), 1, fp) == 0){
    //printf("fwrite error! (1)\n");
  }
}

int main(int argc, char *argv[]) {

  FILE     *fPointer;
	uint8_t  ch[16];
	uint8_t  ch2 = 0;
	uint16_t a, color;
	uint32_t fpos = 0;
	uint8_t  r, g, b;
	uint16_t img_width = 0;
	uint16_t img_height = 0;
	uint8_t  fnameBuffer[512];
  uint8_t  *filenameIn, * filenameOut;
	uint8_t  compression = 1;

  if (argc < 2) {
    printf("usage: %s [input.ppm] (output.p16) (0|1 - compression mode)\n", argv[0]);
    return 0;
  }

	filenameIn = argv[1];

	if (argc > 2) {
	  filenameOut = argv[2];
  } else {
    int i=0;
    while((argv[1])[i] != 0){
      fnameBuffer[i] = (argv[1])[i];
      i++;
    }
    fnameBuffer[i] = 0;
    fnameBuffer[i - 1] = '6';
    fnameBuffer[i - 2] = '1';
    fnameBuffer[i - 3] = 'p';
    printf("setting new fname: %s\n", fnameBuffer);
    filenameOut = fnameBuffer;
  }
  if (argc == 4 && (argv[3])[0]=='0') {
    compression = 0;
  }

  fPointer = fopen(filenameIn, "rb");

	if (!fPointer) {
	  printf("Error while opening file %s!\n", filenameIn);
	  return 1;
	}
  
	ch[0] = svp_fread_u8(fPointer);
	ch[1] = svp_fread_u8(fPointer);
	if ((ch[0] != 'P') && (ch[1] != '6')) {
		printf("draw_ppm: Error: wrong header\n");
		return 1;
	}
	fpos = 3;
	fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
  if(svp_fread_u8(fPointer) == '#') {	
	  while (ch2 != 10) {
	    fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
		  ch2 = svp_fread_u8(fPointer);
		  fpos++;
	  }
	}
  
  ch2 = 0;
  a = 0;
  while (ch2 != 10) {
	  fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
	  ch2 = svp_fread_u8(fPointer);
	  ch[a] = ch2;
	  fpos++;
	  a++;
  }
  ch[a] = 10;
	a = 0;
	while (ch[a] != ' ') {
		img_width *= 10;
		img_width += ch[a] - 48;
		a++;
	}
	a++;
	while (ch[a] != 10) {
		img_height *= 10;
		img_height += ch[a] - 48;
		a++;
	}

	ch2 = 0;

  printf("Image size: w: %u, h: %u \n", img_width, img_height);

	while(ch2 != 10) {
	  fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
		ch2 = svp_fread_u8(fPointer);
		fpos++;
	}

	uint8_t rgtmp[3];
	FILE *fOut;

	fOut = fopen(filenameOut, "wb");

	// header
	svp_fwrite_u8(fOut, 0x1b); // initial non-printable character
	svp_fwrite_u8(fOut, 'P');  // header text
	svp_fwrite_u8(fOut, 'S');
	svp_fwrite_u8(fOut, 'M');
	svp_fwrite_u8(fOut, '1');
	svp_fwrite_u8(fOut, '6');
	svp_fwrite_u8(fOut, 1);    // format version

  // image dimensions
	svp_fwrite_u8(fOut, img_width & 0xFF);
	svp_fwrite_u8(fOut, (img_width & 0xFF00) >> 8);

	svp_fwrite_u8(fOut, img_height & 0xFF);
	svp_fwrite_u8(fOut, (img_height & 0xFF00) >> 8);

	svp_fwrite_u8(fOut, compression); // storege mode
	svp_fwrite_u8(fOut, 1);           // data offset


  int16_t xi = 0;
	int16_t yi = 0;

  if (compression == 0) {
    printf("No compression\n");
    
    for (int i = 0; i < img_width*img_height; i++) {
		  fread(rgtmp, sizeof(rgtmp), 1, fPointer);

		  r = rgtmp[0];
		  g = rgtmp[1];
		  b = rgtmp[2];

		  r = ( r * 249 + 1014 ) >> 11; //5
		  g = ( g * 253 + 505 ) >> 10; //6
		  b = ( b * 249 + 1014 ) >> 11; //5

		  color = r << 11 | (g & 0x3F) << 5 | (b & 0x1F);

	    fwrite(&color, sizeof(color), 1, fOut);
	  }
	}

	if (compression == 1) {
	  printf("Normal compression\n");
    uint16_t prevColor;
    uint16_t prevColor2;
    uint16_t sameDetect = 0;
    uint8_t init = 1;

	  for (int i = 0; i < img_width*img_height; i++) {
		  fread(rgtmp, sizeof(rgtmp), 1, fPointer);
		  r = rgtmp[0];
		  g = rgtmp[1];
		  b = rgtmp[2];

		  r = ( r * 249 + 1014 ) >> 11; //5
		  g = ( g * 253 + 505 ) >> 10; //6
		  b = ( b * 249 + 1014 ) >> 11; //5

		  color = r << 11 | (g & 0x3F) << 5 | (b & 0x1F);

      if (init) {
        fwrite(&color, sizeof(color), 1, fOut);
        prevColor = color;
        init = 0;
        continue;
      }

      if (sameDetect == 0) {
        if (prevColor == color) {
          sameDetect = 1;
        }
	      fwrite(&color, sizeof(color), 1, fOut);
	    } else {
	      if (color == prevColor) {
	        sameDetect++;
	      } else {
	        fwrite(&sameDetect, sizeof(sameDetect), 1, fOut);
	        fwrite(&color, sizeof(color), 1, fOut);
	        sameDetect = 0;
	      }
	    }

	    prevColor = color;
	  }
	}

	fclose(fPointer);
  fclose(fOut);
	return 0;
}
