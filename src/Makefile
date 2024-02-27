CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -g
BUILD_DIR = ../build

TMP = machine.o device.o loader.o disassembler.o debugger.o
OBJECTS = $(patsubst %, $(BUILD_DIR)/%, $(TMP))

.PHONY: clean run

binary: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

clean:
	rm $(BUILD_DIR)/binary $(OBJECTS)
