.PHONY: build
.PHONY: clean

TARGET=kbinput

###########################################################

ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

ifneq (,$(UNION_PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

CC = $(CROSS_COMPILE)g++
CFLAGS = -I../common -DVERSION=\"$(VERSION)\" -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z)
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lrt
OPTM=-O3
STDFLAG = -std=c++17

build:
	$(CC) -o $(BUILD_DIR)/$(TARGET) main.cpp $(CFLAGS) $(LDFLAGS) $(OPTM) $(STDFLAG) -ldl -rdynamic
clean:
	rm -f $(BUILD_DIR)/$(TARGET)
