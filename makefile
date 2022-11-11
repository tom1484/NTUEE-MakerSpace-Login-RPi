CURLPP_LIBS = $(shell pkg-config --cflags --libs curlpp)
JSONCPP_LIBS = $(shell pkg-config --cflags --libs jsoncpp)
OPENCV_LIBS = $(shell pkg-config --cflags --libs opencv4)
WIRING_PI_LIBS = -lwiringPi
THREAD_LIBS = -lpthread

ROOT = .

SRC_PATH = $(ROOT)/src
BUILD_PATH = $(ROOT)/build
BIN_PATH = $(ROOT)/bin

# .PHONY: program

all: main clock frame
	@echo ""

main: 
	g++ $(SRC_PATH)/main.cpp \
		$(SRC_PATH)/serial/serial.cpp \
		$(SRC_PATH)/connection/connection.cpp \
		-o $(BUILD_PATH)/main \
		$(THREAD_LIBS) $(WIRING_PI_LIBS) $(CURLPP_LIBS) $(JSONCPP_LIBS)
	@cp $(BUILD_PATH)/main $(BIN_PATH)/main

frame: 
	g++ $(SRC_PATH)/frame/frame.cpp \
		-o $(BUILD_PATH)/frame \
		$(THREAD_LIBS) $(OPENCV_LIBS) -DOPENCV
	@cp $(BUILD_PATH)/frame $(BIN_PATH)/frame

frame_local: 
	g++ $(SRC_PATH)/frame/frame.cpp \
		-o $(BUILD_PATH)/frame \
		$(THREAD_LIBS)

clock: 
	g++ $(SRC_PATH)/clock/clock.cpp \
		-o $(BUILD_PATH)/clock \
		$(THREAD_LIBS)
	@cp $(BUILD_PATH)/clock $(BIN_PATH)/clock

directories:
	mkdir -p $(BUILD_PATH) $(OBJS_PATH)
