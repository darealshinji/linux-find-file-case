
all: test1 test2

clean:
	-rm -f test1 test2

test1: test1.c
	$(CC) -Wall -O2 -I.. -o $@ $< `pkg-config --cflags --libs glib-2.0`

test2: test2.cpp
	$(CXX) -Wall -O2 -I.. -o $@ $<

