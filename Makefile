CC ?= cc

CFLAGS ?= -O2 -Wall -Wextra -std=c11
LDLIBS = -lxkbcommon

TARGET = wkbmap

SOURCES = \
	src/main.c \
	src/args.c \
	src/xkb.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install:
	install -Dm755 $(TARGET) \
		$(DESTDIR)/usr/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)/usr/bin/$(TARGET)

.PHONY: all clean install uninstall
