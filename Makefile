CC=gcc
CFLAGS=-Wall -g
PTHREAD=-pthread

SRC_DIR=src
UTILS_DIR=$(SRC_DIR)/utils
BIN_DIR=bin
OBJ_DIR=obj

TARGET=$(BIN_DIR)/server
# Include sources from both SRC_DIR and UTILS_DIR
SOURCES=$(wildcard $(SRC_DIR)/*.c $(UTILS_DIR)/*.c)
# Adjust the pattern rule to account for the additional directory level in UTILS_DIR
OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(patsubst $(UTILS_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES)))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(PTHREAD) $^ -o $@

# General rule for compiling .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Specific rule for compiling .c files in UTILS_DIR
$(OBJ_DIR)/%.o: $(UTILS_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/*

.PHONY: all clean