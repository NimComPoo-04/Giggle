include config.mk

CC := gcc

ifeq ($(BUILD), debug)
CFLAGS := -Wall -Wextra -Werror -ggdb -std=c11 -O0
else
CFLAGS := -std=c11 -O2 -DNDEBUG
endif

ifeq ($(TARGET_OS), unix)
CFLAGS += -D_XOPEN_SOURCE -D_POSIX_C_SOURCE=200809L
endif

SRC := $(wildcard *.c $(foreach T, $(DIR), $(T)/*.c))
OBJ := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))

INCDIR := $(addprefix -I, $(INCS))

XLIBDIR := $(addprefix -L, $(LIBDIR))
XLIB := $(addprefix -l, $(LIB))

BLD_DIRS = $(addprefix $(BUILD_DIR)/, $(DIR))

define BLD
mkdir -p $(T)

endef

all: setup $(BUILD_DIR)/$(TARGET)

setup:
	$(foreach T, $(BLD_DIRS), $(BLD))

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(XLIBDIR) $(XLIB)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $(INCDIR) -o $@ $<

run: setup $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

clean:
	$(RM) $(TARGET) $(OBJ)

.PHONY: clean run setup all
