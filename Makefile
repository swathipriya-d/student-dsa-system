CC = gcc
CFLAGS = -Wall -Wextra -O2
SRCS = c_src/linked_list.c c_src/stack.c c_src/queue.c c_src/hash_table.c c_src/bst.c c_src/graph.c c_src/sorting_searching.c c_src/file_io.c c_src/main.c
TARGET = dsa_core

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p data
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) dsa_core.exe
