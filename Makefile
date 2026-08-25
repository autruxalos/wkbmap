CC = cc

CFLAGS = -O2 -Wall -Wextra -std=c11
LDLIBS = -lxkbcommon -lwayland-client

TARGET = wkbmap

SRC = \
	src/main.c \
	src/args.c \
	src/xkb.c \
	src/wayland.c \
	src/wkbmap-protocol.c

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDLIBS) -o $(TARGET)

install: $(TARGET)
	install -Dm755 $(TARGET) /usr/bin/wkbmap

clean:
	rm -f $(OBJ) $(TARGET)
