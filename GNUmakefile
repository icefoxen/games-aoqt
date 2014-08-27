CC = gcc
LINK = gcc

#CC = clang
#LINK = clang

OBJ_BASES := aoqt
OBJS := $(addsuffix .o,$(OBJ_BASES))

CFLAGS := -Wall -std=c99 -D_XOPEN_SOURCE=600 `sdl2-config --cflags`
LDFLAGS := -lm `sdl2-config --libs`

PROGRAM := aoqt

.PHONY: all clean test

all: $(PROGRAM)

clean:
	rm -f $(OBJS) *~ *.o $(PROGRAM) gmon.out

test: $(PROGRAM)
	$(PROGRAM)

# Gotta repeat cflags and ldflags or mingw hates us
$(PROGRAM): $(OBJS)
	$(LINK) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(CFLAGS)

# Now how do we make .c files depend on the appropriate .h files?
