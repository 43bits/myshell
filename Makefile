CC      = gcc
TARGET  = myshell
SRCS    = shell.c parser.c executor.c builtins.c jobs.c signals.c
OBJS    = $(SRCS:.c=.o)

CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 \
          -D_POSIX_C_SOURCE=200809L \
          -g -fsanitize=address,undefined

LDFLAGS = -fsanitize=address,undefined

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "\n Build successful → ./$(TARGET)\n"

%.o: %.c shell.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "🧹 Cleaned build files"

run: all
	./$(TARGET)