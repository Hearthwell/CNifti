SRC_FILES:=$(wildcard src/*.c)
OBJ_FILES:=$(SRC_FILES:src/%.c=out/%.c.o)

C_FLAGS:=-Wall -Wextra -g
C_INC:=-I./include
C_LNK:=-lm

all: ex-example1 nifti

ex-example1: out/example1.c.o $(OBJ_FILES)
	gcc $^ -o $@ $(C_LNK)

nifti: $(OBJ_FILES)
	ar rcs libnifti.a $^

out/third_party.c.o:src/third_party.c
	gcc $(C_INC) -c $< -o $@

out/%.c.o:src/%.c
	gcc $(C_FLAGS) $(C_INC) -c $< -o $@

out/%.c.o:examples/%.c
	gcc $(C_FLAGS) $(C_INC) -c $< -o $@

clean:
	rm -rf out/*
	rm -f libnifti.a ex-example1