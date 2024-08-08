CC = gcc
CFLAGS = -O0 -march=native -pipe -Wall -lm
NAME = main
NAME_MODULE = module

run:
	clear
	$(CC) $(NAME).c $(NAME_MODULE).c -o $(NAME) $(CFLAGS)
	./$(NAME)