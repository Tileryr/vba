CC=g++
CFLAGS=-pedantic -g  -I. -O2 -fsanitize=address

SRC=$(wildcard src/*.cpp) $(wildcard src/cpu/*.cpp) $(wildcard src/cpu/opcodes/*.cpp) $(wildcard src/cpu/opcodes/arm/*.cpp)
OBJ=$(SRC:%.cpp=%.o)
DEP=$(OBJ:%.o=%.d)

EXE=vba
LIBS=$(addprefix -l,) `pkg-config --libs --cflags sdl3`

$(EXE): $(OBJ) 
	$(CC) -fsanitize=address -static-libasan -g -o $@ $^ $(LIBS)

-include $(DEP)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(DEP) $(EXE)