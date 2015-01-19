# bryte makefile

# directories
SOURCE_DIR = ../source

# vars
CC             = g++
CFLAGS         = -Wall -Werror -m64 -fPIC -std=c++11
INCLUDE        = -I$(SOURCE_DIR)
LINK           = -lSDL2 -ldl
GAME_SO        = bryte_game.so
GAME_SO_OBJS   = Log.o Utils.o Bitmap.o Map.o Interactives.o Character.o Enemy.o MapDisplay.o Camera.o
GAME           = bryte
EDITOR_SO      = bryte_editor.so
EDITOR_SO_OBJS = Log.o Utils.o Map.o Bitmap.o Text.o MapDisplay.o
EDITOR         = bryte_editor
EXE_OBJS       = Log.o InputRecorder.o GameInput.o Application.o

# targets
all: release
release: CFLAGS += -O3
release: $(GAME_SO) $(GAME) $(EDITOR_SO) $(EDITOR)
debug: CFLAGS += -g3 -DDEBUG
debug: $(GAME_SO) $(GAME) $(EDITOR_SO) $(EDITOR)

# rules
clean:
	rm -f $(EXE_OBJS) $(GAME_SO) $(GAME_SO_OBJS) $(GAME) $(EDITOR_SO) $(EDITOR_SO_OBJS) $(EDITOR)

$(GAME_SO): $(GAME_SO_OBJS) $(SOURCE_DIR)/Bryte.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $^ -shared -o $@ $(LINK)

$(GAME): $(EXE_OBJS) $(SOURCE_DIR)/BryteMain.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LINK)

$(EDITOR_SO): $(EDITOR_SO_OBJS) $(SOURCE_DIR)/Editor.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $^ -shared -o $@ $(LINK)

$(EDITOR): $(EXE_OBJS) $(SOURCE_DIR)/EditorMain.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LINK)

%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

