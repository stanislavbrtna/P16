all:
	gcc ppmtop16.c -o bin/ptp16

test: all
	./bin/ptp16 images/lenna.ppm images/out/lenna_nc.p16 0
	./bin/ptp16 images/lenna.ppm images/out/lenna_c1.p16 1
	
