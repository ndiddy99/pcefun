CC = gcc

TARGET = pceconv.exe
CFLAGS = -g -std=gnu99 -Wall

OBJECTS = pceconv.o qdbmp.o tile.o sprite.o

all: $(TARGET)

clean:
	rm *.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<