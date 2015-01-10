# bryte makefile

# directories
SOURCE_DIR = ../source

# vars
CC             = g++
CFLAGS         = -Wall -Werror -m64 -fPIC -std=c++11
INCLUDE        = -I$(SOURCE_DIR)
LINK           = -lSDL2 -ldl
GAME_SO        = bryte_game.so
GAME_SO_OBJS   = Log.o BryteGlobals.o Map.o Utils.o Bitmap.o Camera.o
EDITOR_SO      = bryte_editor.so
EDITOR_SO_OBJS = Log.o
EXE            = bryte
EXE_OBJS       = Log.o InputRecorder.o GameInput.o Application.o

all: release
release: CFLAGS += -O3
release: $(GAME_SO) $(EDITOR_SO) $(EXE)
debug: CFLAGS += -g3 -DDEBUG
debug: $(GAME_SO) $(EDITOR_SO) $(EXE)

clean:
	rm -f $(GAME_SO) $(GAME_SO_OBJS) $(EDITOR_SO) $(EDITOR_SO_OBJS) $(EXE_OBJS) $(EXE)

$(GAME_SO): $(GAME_SO_OBJS) $(SOURCE_DIR)/Bryte.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(GAME_SO_OBJS) $(SOURCE_DIR)/Bryte.cpp -shared -o $@ $(LINK)

$(EDITOR_SO): $(EDITOR_SO_OBJS) $(SOURCE_DIR)/Bryte.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(EDITOR_SO_OBJS) $(SOURCE_DIR)/Editor.cpp -shared -o $@ $(LINK)

$(EXE): $(EXE_OBJS) $(SOURCE_DIR)/Main.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(EXE_OBJS) $(SOURCE_DIR)/Main.cpp -o $@ $(LINK)

%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

