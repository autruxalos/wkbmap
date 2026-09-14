CC = cc

CFLAGS = -O2 -Wall -Wextra -std=c11
LDLIBS = -lxkbcommon -lwayland-client

TARGET = wkbmap

SRC = \
	src/main.c \
	src/args.c \
	src/xkb.c \
	src/wkbmap-protocol.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDLIBS) -o $@

install: $(TARGET)
	install -Dm755 $(TARGET) /usr/bin/$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all install clean
