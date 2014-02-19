CC=g++
CFLAGS=-W -pedantic -std=c++0x

siglot_test: example.cpp
	$(CC) -o $@ $(CFLAGS) $^