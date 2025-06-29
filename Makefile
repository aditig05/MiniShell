CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lreadline -lhistory -ltermcap

SRC = main.c shell.c parser.c commands.c natural_commands.c ai_suggest.c
OBJ = $(SRC:.c=.o)
TARGET = myshell

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)