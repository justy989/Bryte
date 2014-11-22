# bryte makefile

# directories
SOURCE_DIR      = ../source
SDL_INCLUDE_DIR = /usr/local/include
SDL_LINK_DIR    = /usr/local/lib

# vars
CC      = g++
CFLAGS  = -Wall -Werror -std=c++11
INCLUDE = -I$(SDL_INCLUDE_DIR) -I$(SOURCE_DIR)
LINK    = -L$(SDL_LINK_DIR) -lSDL2 -lSDL2main
EXE     = bryte
OBJS    = sprite.o animated_sprite.o clipped_sprite.o room.o sdl.o sdl_window.o\
          configuration.o surface_man.o camera.o draw_utils.o ui_button.o\
          ui_icon_button.o ui_text_button.o mouse.o room_display.o text.o\
          title_state.o editor_state.o world_state.o game.o

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


