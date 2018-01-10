TARGET_EXEC := gm_detect

BUILD_DIR := ./build
SRC_DIRS  := ./src

CC = g++
CFLAGS = -c -Wall -std=c++11 -O1 -I"/usr/local/include/opencv" -I"/usr/local/include/opencv2" -I"/usr/include"
LDFLAGS = -L"/usr/local/lib"
LIBS = -I"/usr/local/include/opencv" -I"/usr/local/include/opencv2" -I"/usr/include" -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_imgcodecs -lopencv_videoio

SRCS := $(shell	find $(SRC_DIRS)	-name	*.cpp	-or	-name	*.c	-or	-name	*.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS := -MMD -MP

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ 

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR) $(TARGET_EXEC)

-include $(DEPS)

MKDIR_P := mkdir -p
