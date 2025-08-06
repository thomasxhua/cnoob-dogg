CXX = gcc
CPPFLAGS = -g -O0
CXXFLAGS = -std=c99 -Wall -Wextra -pedantic -static
LDFLAGS  =

SRC = \
	src/utils.c \
	src/piece.c \
	src/bitboard.c \
	src/board_state.c \
	src/main.c \

MAIN = dist/main.exe

all: $(MAIN)

$(MAIN): $(SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	del /Q dist\*

