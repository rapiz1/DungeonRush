OBJS = *.c
CC = gcc
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
CFLAGS = -g -Wall
OBJ_NAME = snake

snake: $(OBJS) *.h
	$(CC) $(OBJS) $(LINKER_FLAGS) $(CFLAGS) -o $(OBJ_NAME) -DDBG
cross: $(OBJS) *.h
	$(CC) $(OBJS) $(LINKER_FLAGS) $(CFLAGS) -o $(OBJ_NAME) -DDBG -DDBG_CROSS
dist: $(OBJS) *.h
	$(CC) $(OBJS) $(LINKER_FLAGS) -O2 -o $(OBJ_NAME)
zip:
	zip -r snake$(date -I) *.c *.h Makefile res
run: snake
	./snake