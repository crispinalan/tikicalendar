CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk4)
LIBS = $(shell $(PKGCONFIG) --libs gtk4) -lm -lasound

SRC = main.c

OBJS = $(SRC:.c=.o)

all: tikicalendar

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<


tikicalendar: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -f tikicalendar
