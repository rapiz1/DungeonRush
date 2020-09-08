CC = gcc
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net -lm
CFLAGS = -g -Wall

SRC = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJS = ${SRC:.c=.o}

EXEC = dungeon_rush

.PHONEY = clean

$(EXEC): $(OBJS) $(DEPS)
	$(CC) $(OBJS) $(LDFLAGS) $(CFLAGS) -o $(EXEC) -DDBG

clean:
	rm $(OBJS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dist_bin: $(OBJS) src/*.h
	$(CC) $(OBJS) $(LDFLAGS) -O2 -o $(EXEC)

dist_linux: dist_bin
	mkdir -p dist/linux
	cp $(EXEC) dist/linux
	cp -r res dist/linux
	zip -r dist/DungeonRush_linux.zip dist/linux

zip:
	zip -r dungeon_rush`date -I` *.c *.h Makefile res *.dat
	cp dungeon_rush`date -I`.zip ~/Downloads
	mv *.zip archive

run: dungeon_rush
	./dungeon_rush
