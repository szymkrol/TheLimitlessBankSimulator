CC     = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude

SRC = src/main.c src/client.c src/server.c src/communication.c src/db_manager.c
OBJ = $(SRC:.c=.o)
BIN = TheLimitlessBankSimulator

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean