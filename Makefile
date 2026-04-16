CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = dots_and_boxes

SRC_DIR = src
INC_DIR = include

OBJS = $(SRC_DIR)/main.o $(SRC_DIR)/game.o $(SRC_DIR)/board.o $(SRC_DIR)/player.o $(SRC_DIR)/bot.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/main.c -o $(SRC_DIR)/main.o

$(SRC_DIR)/game.o: $(SRC_DIR)/game.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/game.c -o $(SRC_DIR)/game.o

$(SRC_DIR)/board.o: $(SRC_DIR)/board.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/board.c -o $(SRC_DIR)/board.o

$(SRC_DIR)/player.o: $(SRC_DIR)/player.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/player.c -o $(SRC_DIR)/player.o

$(SRC_DIR)/bot.o: $(SRC_DIR)/bot.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/bot.c -o $(SRC_DIR)/bot.o

run: $(TARGET)
	./$(TARGET)
	
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

.PHONY: clean
