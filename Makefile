CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -std=c11
LDFLAGS =
SRC = main.c vm.c trap.c hook.c util.c
OBJ = $(SRC:.c=.o)
TARGET = ghostvisor

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
