CFLAGS = -Wall -Wextra -O3


all: test1 test2

clean:
	-rm -f test1 test2

test1: test1.c
	$(CC) $(CFLAGS) -I.. -o $@ $< `pkg-config --cflags --libs glib-2.0`

test2: test2.cpp
	$(CXX) $(CFLAGS) -I.. -o $@ $<

