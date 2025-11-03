# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall  -std=c++11 -Iinclude

# Directories
INCDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
SRC_FILES = $(wildcard $(SRCDIR)/*.cpp)
OBJ_FILES = $(SRC_FILES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Executable files
EXECUTABLES = makeFileSystem fileSystemOper

# Targets
all: $(EXECUTABLES)

makeFileSystem: $(OBJDIR)/file_system.o $(OBJDIR)/makeFileSystem.o
	
	$(CXX) $^ -o $@

fileSystemOper: $(OBJDIR)/file_system.o $(OBJDIR)/fileSystemOper.o
	
	$(CXX) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.hpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/makeFileSystem.o: makeFileSystem.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c makeFileSystem.cpp -o $@

$(OBJDIR)/fileSystemOper.o: fileSystemOper.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c fileSystemOper.cpp -o $@

clean:
	rm -rf $(OBJDIR) makeFileSystem fileSystemOper mySystem.dat

.PHONY: all clean
