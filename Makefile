CFLAGS = -Wall -g -pthread
LDFLAGS = -lm

.PHONY: all clean

all: main

main: *.cpp
	g++ $(CFLAGS) *.cpp -o main $(LDFLAGS)

clean:
	rm -f main