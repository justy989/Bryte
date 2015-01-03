# bryte makefile

# directories
SOURCE_DIR = ../source

# vars
CC       = g++
CFLAGS   = -Wall -Werror -m64 -fPIC -std=c++11
INCLUDE  = -I$(SOURCE_DIR)
LINK     = -lSDL2 -ldl
SO       = bryte_game.so
SO_OBJS  = Log.o Globals.o Map.o
EXE      = bryte
EXE_OBJS = Log.o InputRecorder.o GameInput.o Application.o

all: release
release: CFLAGS += -O3
release: $(SO) $(EXE)
debug: CFLAGS += -g3 -DDEBUG
debug: $(SO) $(EXE)

clean:
	rm -f $(SO) $(SO_OBJS) $(EXE_OBJS) $(EXE)

$(SO): $(SO_OBJS) $(SOURCE_DIR)/Bryte.cpp
	$(CC) -shared $(CFLAGS) $(INCLUDE) $(SO_OBJS) $(SOURCE_DIR)/Bryte.cpp -o $@ $(LINK)

$(EXE): $(EXE_OBJS) $(SOURCE_DIR)/Main.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(EXE_OBJS) $(SOURCE_DIR)/Main.cpp -o $@ $(LINK)

%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

