PROG_NAME   = $(notdir $(CURDIR))
DIST_DIR    = ./dist

SRC_DIR     = ./src
BUILD_DIR   = ./build
INCLUDE_DIR = ./include

CFLAGS       = -Wall -O3 -std=c++11 -fpermissive -Wno-unknown-pragmas
LIBS        = -L lib/ -fopenmp

CC          = g++
LD          = g++

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC_LIST = $(call rwildcard, $(SRC_DIR), *.cpp)
OBJ_LIST = $(addsuffix .o, $(basename $(patsubst %, $(BUILD_DIR)/%, $(SRC_LIST:$(SRC_DIR)/%=%))))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $(CFLAGS) -I $(SRC_DIR) -o $@ $<
	
#compile and link
default: $(PROG_NAME)

#compile without linking
compile: $(OBJ_LIST)

#compile and link
$(PROG_NAME): compile
	$(LD) $(OBJ_LIST) $(LIBS) -o $(DIST_DIR)/$@

clean:
	rm -f $(DIST_DIR)/$(PROG_NAME) $(BUILD_DIR)/*.o