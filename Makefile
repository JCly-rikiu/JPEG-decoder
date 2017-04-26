COMPILER = clang++

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
INC_DIR = ./include

TARGET = $(BIN_DIR)/jpeg_decoder

SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cc=.o)))

LIBS =
INCLUDE   = -I$(INC_DIR)
CPPFLAGS += -Wall
LDFLAGS +=
LDLIBS +=

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(COMPILER) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	$(RM) $(OBJS)

run:
	$(TARGET)
