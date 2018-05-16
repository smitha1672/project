CC = gcc
TARGET_EXEC ?= a.out

BUILD_DIR ?= ./firmware/build
SRC_DIRS ?= ./firmware

SRCS += ./firmware/src/main.c
#SRCS += ./firmware/src/heap.c
SRCS += ./firmware/src/sensors.c
SRCS += ./firmware/src/sensor_manager/src/sensor_manager.c


OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS += -I./firmware/inc/
INC_FLAGS += -I./firmware/inc/cpu/x86/
INC_FLAGS += -I./firmware/src/sensor_manager/inc/

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
CFLAGS ?= $(INC_FLAGS) -lpthread -O3 -Werror -lm
CFLAGS += -DFORCE_HEAP_IN_DOT_DATA
CFLAGS += -DHEAP_SIZE=2048
LDFLAGS += $(CFLAGS) $(CPPFLAGS)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
	MKDIR_P ?= mkdir -p
