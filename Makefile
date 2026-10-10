CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -O2 -MMD -MP -g -Wno-missing-field-initializers -Wno-unused-result
LDLIBS := -lm -g

TARGET := hinge-driver

SRC := hinge-driver.c accel.c vec.c uinput-thing.c
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

clean:
	rm -f $(OBJ) $(DEP) $(TARGET)