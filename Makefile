CC=gcc
CFLAGS=-Wall -Wextra -g
SRC=main.c app.c router.c layer.c response.c request.c error.c route.c json.c form.c
OBJ=$(SRC:.c=.o)
DEPS=app.h router.h route.h layer.h response.h request.h error.h json.h form.h
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

test_route_metadata: test_route_metadata.c $(filter-out main.o,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^

test_json_parsing: test_json_parsing.c $(filter-out main.o,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^

test_json_simple: test_json_simple.c json.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all clean
