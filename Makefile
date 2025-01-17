# Compiler
CC = clang

# Compiler flags
CFLAGS = -std=c99 -Wall -pedantic -fpic $(shell python3-config --cflags)

# Linker flags
LDFLAGS = -shared $(shell python3-config --ldflags)

# SWIG flags
SWIG = swig
SWIG_FLAGS = -python

# Source and Header files
SRC = hclib.c
HEADER = hclib.h
INTERFACE = hclib.i

# Target names
TARGET_LIB = libhclib.so
TARGET_PY_LIB = _hclib.so
OBJECT = hclib.o
WRAP_OBJECT = hclib_wrap.o



# Default target: Builds everything
all: $(TARGET_LIB) $(TARGET_PY_LIB)

# Rule for creating hclib.o (position-independent code)
$(OBJECT): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJECT)

# Rule for creating the shared library libhclib.so
$(TARGET_LIB): $(OBJECT)
	$(CC) $(LDFLAGS) -o $(TARGET_LIB) $(OBJECT)

# Rule for generating SWIG interface files (hclib_wrap.c and hclib.py)
hclib_wrap.c hclib.py: $(INTERFACE) $(HEADER)
	$(SWIG) $(SWIG_FLAGS) $(INTERFACE)

# Rule for creating hclib_wrap.o from hclib_wrap.c
$(WRAP_OBJECT): hclib_wrap.c $(HEADER)
	$(CC) $(CFLAGS) -c hclib_wrap.c -o $(WRAP_OBJECT)

# Rule for creating the Python shared object library _hclib.so
$(TARGET_PY_LIB): $(WRAP_OBJECT) $(OBJECT)
	$(CC) $(LDFLAGS) -o $(TARGET_PY_LIB) $(WRAP_OBJECT) $(OBJECT)

# Clean up: removes all .o, .so, .c, .py, and any executables
clean:
	rm -f $(OBJECT) $(TARGET_LIB) $(WRAP_OBJECT) $(TARGET_PY_LIB) hclib_wrap.c hclib.py