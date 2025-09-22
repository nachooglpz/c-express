CC=gcc
CFLAGS=-Wall -Wextra -g
SRC=main.c app.c router.c layer.c response.c request.c
OBJ=$(SRC:.c=.o)
DEPS=app.h router.h route.h layer.h response.h request.h
TARGET=c-server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
