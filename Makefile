CC = g++
CFLAGS = -Wall -std=c++11 -c

TARGET = main
SOURCES = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS)  $(SOURCES)
	$(CC)  main.o -o $(TARGET) -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f $(TARGET)
	


