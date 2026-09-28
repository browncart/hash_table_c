SRC = 	src/hash_table.c	\
		src/string.c		

MAIN 	= src/main.c
OUT 	= bin/main
CFLAGS 	= -std=c99

run:
	gcc $(CFLAGS) $(FLAGS) $(MAIN) $(SRC) -o $(OUT)