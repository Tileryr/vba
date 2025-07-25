CC=g++
CFLAGS=-pedantic

SRC=$(wildcard src/*.cpp) $(wildcard src/cpu/*.cpp) $(wildcard src/cpu/opcodes/*.cpp)
OBJ=$(SRC:%.cpp=%.o)
DEP=$(OBJ:%.o=%.d)

EXE=vba
LIBS=$(addprefix -l,)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(DEP) $(EXE)