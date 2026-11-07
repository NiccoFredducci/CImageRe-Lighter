CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -O2
CPPFLAGS = -Isrc -Isrc/passes
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
PASS_DIR = $(SRC_DIR)/passes
BUILD_DIR = build

SOURCES = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/pipeline.c \
	$(PASS_DIR)/depth_pass.c

OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

TARGET = $(BUILD_DIR)/relight_studio.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)