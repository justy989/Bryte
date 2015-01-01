# bryte makefile

# directories
SOURCE_DIR      = ../source
SDL_INCLUDE_DIR = /usr/include
SDL_LINK_DIR    = /usr/lib64

# vars
CC       = g++
CFLAGS   = -Wall -Werror -m64 -fPIC -std=c++11
INCLUDE  = -I$(SDL_INCLUDE_DIR) -I$(SOURCE_DIR)
LINK     = -L$(SDL_LINK_DIR) -lSDL2 -ldl
SO       = bryte_game.so
EXE      = bryte
EXE_OBJS = Platform.o

all: release
release: CFLAGS += -O3 -DNDEBUG
release: $(SO) $(EXE)
debug: CFLAGS += -g3
debug: $(SO) $(EXE)

clean:
	rm -f $(SO) $(EXE_OBJS) $(EXE)

$(SO): $(SOURCE_DIR)/Bryte.cpp
	$(CC) -shared $(CFLAGS) $(INCLUDE) $(SOURCE_DIR)/Bryte.cpp -o $@ $(LINK)

$(EXE): $(EXE_OBJS) $(SOURCE_DIR)/Main.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(EXE_OBJS) $(SOURCE_DIR)/Main.cpp -o $@ $(LINK)

%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

