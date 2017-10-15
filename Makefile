CC=gcc
CXX=g++
CFLAGS=-Wall -O0 -g -fPIC `pkg-config sdl openal freealut --cflags` -DHAVE_OPENAL -DHAVE_SDL -DDEBUG
LDFLAGS=`pkg-config sdl openal freealut --libs` -Ltarget/debug/
LIBS=-lclient -lserver -lmenu -lm -ldl -lstdc++ -lGL -lGLU -lGLEW

VPATH=src/client/ src/shared/ src/server/ src/menu/ src/cognition/

SHARED_OBJS=$(patsubst src/shared/%.c, target/debug/%.o, $(wildcard src/shared/*.c))
CLIENT_OBJS=$(patsubst src/client/%.c, target/debug/%.o, $(wildcard src/client/*.c))
SERVER_OBJS=$(patsubst src/server/%.c, target/debug/%.o, $(wildcard src/server/*.c))
MENU_OBJS=$(patsubst src/menu/%.cpp, target/debug/%.o, $(wildcard src/menu/*.cpp))
COG_OBJS=$(patsubst src/cognition/%.c, target/debug/%.o, $(wildcard src/cognition/*.c))

all: debug

debug: target/debug/cognition

target/debug/cognition: target/debug/libserver.so target/debug/libclient.so target/debug/libmenu.so $(COG_OBJS) $(SHARED_OBJS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(LIBS) $^

target/debug/libmenu.so: $(MENU_OBJS) $(SHARED_OBJS)
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^

target/debug/libserver.so: $(SERVER_OBJS) $(SHARED_OBJS)
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^

target/debug/libclient.so: $(CLIENT_OBJS) $(SHARED_OBJS)
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^

target/debug/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

target/debug/%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf target/debug/*
