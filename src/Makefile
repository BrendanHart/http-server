SRC_DIR = ./
OBJ_DIR = ../obj
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

CC=clang

EXE=../server
CFLAGS=-Wall -Werror
LDFLAGS=-lssl -lcrypto -pthread

all: $(OBJ_DIR) $(EXE)

$(EXE): $(OBJ_FILES)
	$(CC) -o $(EXE) $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	$(CC) $< -c -o $@ $(CFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
