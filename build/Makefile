# bryte makefile

# directories
SOURCE_DIR = ../source

# vars
#CC             = /home/justin/.vim/bundle/clang_complete/bin/cc_args.py clang++
CC             = g++
CFLAGS         = -Wall -Werror -fPIC -std=c++11 -DLINUX
INCLUDE        = -I$(SOURCE_DIR)
LINK           = -lSDL2 -lSDL2_mixer -ldl
GAME_SO        = bryte_game.so
GAME_SO_OBJS   = Log.o Utils.o Bitmap.o Region.o Map.o Interactives.o Character.o Player.o Enemy.o \
                 Pickup.o Projectile.o Bomb.o MapDisplay.o CharacterDisplay.o InteractivesDisplay.o \
                 PickupDisplay.o ProjectileDisplay.o Emitter.o Camera.o Dialogue.o Text.o Sound.o
GAME           = bryte
EDITOR_SO      = bryte_editor.so
EDITOR_SO_OBJS = Log.o Utils.o Map.o Character.o Interactives.o Pickup.o Bitmap.o Text.o MapDisplay.o \
			  CharacterDisplay.o InteractivesDisplay.o
EDITOR         = bryte_editor
EXE_OBJS       = Log.o InputRecorder.o GameFunction.o GameInput.o Application.o

# targets
all: debug
release: CFLAGS += -O3
release: $(GAME_SO) $(GAME) $(EDITOR_SO) $(EDITOR)
debug: CFLAGS += -g3 -DDEBUG
debug: $(GAME_SO) $(GAME) $(EDITOR_SO) $(EDITOR)
cygwin: LINK = -L/usr/local/lib -lcygwin -lSDL2main -lSDL2 -mwindows -ldl
cygwin: CFLAGS = -Wall -Werror -std=c++11 -DLINUX
cygwin: INCLUDE += -I/usr/local/include
cygwin: debug

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

