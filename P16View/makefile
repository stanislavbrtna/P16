all:
	gcc p16view.c p16util.c SDA_fs/sda_fs_pc.c  -lSDL2 -o bin/p16v

test: all
	./bin/p16w bin/Images/lenna.p16

