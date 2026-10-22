CC = g++
CFLAGS = -Wall -g

.PHONY: clean build

build: command.cc tokenizer.cc command.h tokenizer.h
	$(CC) $(CFLAGS) -o myshell command.cc tokenizer.cc

clean:
	rm -f myshell