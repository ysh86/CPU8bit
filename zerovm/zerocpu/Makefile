TARGET_LIB ?= libzerocpu.a

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find -L $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find -L $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# pkgconfig
# none

CPPFLAGS := $(INC_FLAGS) -MMD -MP -D_POSIX_C_SOURCE=200809L
CPPFLAGS += $(PKG_CONFIG_CFLAGS)
LDFLAGS += $(PKG_CONFIG_LIBS)

CFLAGS += -Wall --std=c99 -O3
CXXFLAGS += -Wall --std=c++11 -O3


$(BUILD_DIR)/$(TARGET_LIB): $(OBJS)
	$(MKDIR_P) $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ $(OBJS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	$(RM) -r $(OBJS) $(DEPS) $(BUILD_DIR)

-include $(DEPS)

RM ?= rm -f
MKDIR_P ?= mkdir -p
