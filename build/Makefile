# bryte makefile

# directories
SOURCE_DIR      = ../source
SDL_INCLUDE_DIR = /usr/include
SDL_LINK_DIR    = /usr/lib64

# vars
CC      = g++
CFLAGS  = -Wall -Werror -std=c++11
INCLUDE = -I$(SDL_INCLUDE_DIR) -I$(SOURCE_DIR)
LINK    = -L$(SDL_LINK_DIR) -lSDL2
EXE     = bryte
OBJS    = Platform.o

all: release
release: CFLAGS += -O3 -DNDEBUG
release: $(EXE)
debug: CFLAGS += -g3
debug: $(EXE)

clean:
	rm -f $(OBJS) $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJS) $(SOURCE_DIR)/main.cpp -o $@ $(LINK)

%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

