CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lssl -lcrypto
SRC = src/main.c src/encryption.c src/interface.c
OBJ = src/main.o src/encryption.o src/interface.o
TARGET = build/my_program

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)