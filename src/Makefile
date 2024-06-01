CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

# Object directory
OBJDIR = obj

# Source files
SRC1 = src/makeFileSystem.c src/file_system.c src/shell_commands.c
SRC2 = src/fileSystemOper.c src/file_system.c src/shell_commands.c

# Object files in the obj directory
OBJ1 = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC1))
OBJ2 = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC2))

# Executable names
EXEC1 = makeFileSystem
EXEC2 = fileSystemOper

all: $(EXEC1) $(EXEC2)

# Rule to build the first executable
$(EXEC1): $(OBJ1)
	$(CC) $(LDFLAGS) $(OBJ1) -o $@

# Rule to build the second executable
$(EXEC2): $(OBJ2)
	$(CC) $(LDFLAGS) $(OBJ2) -o $@

# Rule to compile source files into object files in obj directory
$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Valgrind targets
valgrind-makeFileSystem: $(EXEC1)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC1)

valgrind-fileSystemOper: $(EXEC2)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC2)

clean:
	rm -f $(EXEC1) $(EXEC2) $(OBJDIR)/*.o mySystem.dat
	rm -rf $(OBJDIR)

.PHONY: all clean valgrind-makeFileSystem valgrind-fileSystemOper
