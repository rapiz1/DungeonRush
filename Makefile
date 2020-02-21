OBJS = *.c
CC = gcc
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
CFLAGS = -g -Wall
OBJ_NAME = snake

snake: $(OBJS) *.h
	$(CC) $(OBJS) $(LINKER_FLAGS) $(CFLAGS) -o $(OBJ_NAME) -DDBG
	echo 1 0 0 0 0 0 > storage.dat
dist_bin: $(OBJS) *.h
	$(CC) $(OBJS) $(LINKER_FLAGS) -O2 -o $(OBJ_NAME)
	echo 1 0 0 0 0 0 > storage.dat
dist_linux: dist_bin
	mkdir -p dist/linux
	cp $(OBJ_NAME) dist/linux
	cp -r res dist/linux
	cp storage.dat dist/linux
	zip -r dist/DungeonRush_linux.zip dist/linux
zip:
	zip -r snake`date -I` *.c *.h Makefile res *.dat
	cp snake`date -I`.zip ~/Downloads
	mv *.zip archive
run: snake
	./snake
