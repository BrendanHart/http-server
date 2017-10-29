SRC_DIR = ./
OBJ_DIR = ../obj
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

CC=clang

EXE=../server
CFLAGS=-Wall -Werror
LDFLAGS=-pthread -lssl -lcrypto

all: $(OBJ_DIR) $(EXE)

$(EXE): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(EXE) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
