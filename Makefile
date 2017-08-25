MKDIR=mkdir -p
C=gcc
CFLAGS=-std=c99 -c -Wall -Iinclude/
LDFLAGS=
SOURCES=$(wildcard src/*.c)
OBJECTS=$(addprefix build/, $(notdir $(SOURCES:.c=.o)))
EXECUTABLE=bin/omong
MAKEDIRS=bin/ build/

.PHONY: clean

all: $(MAKEDIRS) $(SOURCES) $(EXECUTABLE)

$(MAKEDIRS):
	${MKDIR} $(MAKEDIRS)

clean:
	rm -rf build/*.o bin/*

$(EXECUTABLE): $(OBJECTS)
	$(C) $(LDFLAGS) $(OBJECTS) -o $@ -lm

build/%.o: src/%.c
	$(C) $(CFLAGS) $< -o $@
