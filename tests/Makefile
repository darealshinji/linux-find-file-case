CFLAGS    = -Wall -Wextra -O3
TEST_BINS = test1 test2 test3


all: $(TEST_BINS)

clean:
	-rm -f $(TEST_BINS)

test1: test1.c
	$(CC) $(CFLAGS) -I.. -o $@ $< `pkg-config --cflags --libs glib-2.0`

test2: test2.cpp
	$(CXX) $(CFLAGS) -I.. -o $@ $<

test3: test3.cpp
	$(CXX) $(CFLAGS) -std=c++17 -I.. -o $@ $<

