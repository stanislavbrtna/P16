## P16 - Simple 16bit image format
 >version 1

Simple image format for use with low performance devices, mainly for storage of system icons etc. In basic compression mode can greatly reduce size of images with large areas of same color pixels. It is intended to use it with some filesystem.

### Usage and performance
Uncompressed images are 2/3 size of the same image in PPM, because of native 16bit colors.
Compressed images can further reduce overall file size to about 1/2 of the uncompressed file. This is content-dependent and in the worst case the compressed image is larger than the uncompressed, in that case disable the compression when encoding the images.
P16 is used as the default image format for the SDA_OS.
### Spec:
|  Byte no.: | Value | Description: |
|--|--|--|
| 0 | 0x1b | Initial byte |
| 1 - 5 | PSM16 | ASCII: PSM16 |
| 6 | 1 |  Format version: (1 for this spec) |
| 7 |  | Image width LSB |
| 8 |  | Image width MSB |
| 9 |  | Image height LSB |
| 10 |  | Image height MSB |
| 11 |  | storage mode 0 - uncompressed 1 - basic compression |
| 12 |  | Data section offset |

Remaining 255 or so bytes of offset could be used for some additional info. Otherwise set the offset to 1 and start the data section on byte 13.
### Data section:
  Field of 16 bit values interpreted depending upon the storage mode.
#### Normal mode
  In normal mode (0) those are 16bit RGB565 values for each pixel of the screen.
#### Compressed mode
  In compressed mode (1) if two pixels in a row have the same color, the third 16bit value  is count of how many more pixels have the same color, if there is zero, then no more  same colored pixels will be drawn. This method of compression is super simple and performs greatly if there are large areas of pixels with the same color,  but if there are many occurrences of just two pixels with the same color, the final storage  size of those pixels will be larger than when left uncompressed.
  Anyway, even in a worst case scenario this compression method can not create  file larger than the original PPM file, so that is good.

### Reference implementation
If my implementation is working according to this spec, then it is the reference, but I never actually checked that. Anyway this repo contains tool for converting gimp-generated ppm to p16 and p16view utility.
