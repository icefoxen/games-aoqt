CC = gcc
LINK = gcc

#CC = clang
#LINK = clang

OBJ_BASES := main
OBJS := $(addsuffix .o,$(OBJ_BASES))

CFLAGS := -O3 -I include -I/usr/local/include -Wall -std=c99 -L/usr/local/lib -Wl,-rpath=/usr/local/lib -lSDL2
CCF = $(CC) $(CFLAGS)

PROGRAM := aoqt

.PHONY: all clean test

all: $(PROGRAM)

clean:
	rm -f $(OBJS) *~ *.o $(PROGRAM) gmon.out

test: $(PROGRAM)
	$(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LINK) $(CFLAGS) -o $(PROGRAM) $(OBJS)

%.o: %.c
	$(CCF) -c $<

# Now how do we make .c files depend on the appropriate .h files?
