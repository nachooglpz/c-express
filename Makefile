CC=gcc
CFLAGS=-Wall -Wextra -g
SRC=main.c app.c router.c layer.c response.c request.c error.c route.c
OBJ=$(SRC:.c=.o)
DEPS=app.h router.h route.h layer.h response.h request.h error.h
TARGET=c-server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

# Test targets
test_validation_simple: test_validation_simple.c $(filter-out main.o,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^

test_pattern: test_pattern.c $(filter-out main.o,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^

test_subrouters: test_subrouters.c $(filter-out main.o,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all clean
