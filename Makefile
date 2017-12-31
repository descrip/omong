TARGET_EXEC ?= omong
BUILD_DIR ?= ./build
SRC_DIR ?= ./src

SOURCES := $(shell find $(SRC_DIRS) -name *.cc)
OBJECTS := $(SOURCES:%.cc=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

CC := g++-5
CCFLAGS := -std=c++14 -Wall -MMD -g -Wold-style-cast # -O2
LDFLAGS :=

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cc
	$(MKDIR_P) $(dir $@)
	$(CC) $(CCFLAGS) $(CCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPENDS)

MKDIR_P ?= mkdir -p

