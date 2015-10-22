CXX=g++
CPPFLAGS=-I. -O2
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
TARGET=convert1

.PHONY: all clean
	
all: .d $(SOURCES) $(TARGET)
	
.d: $(SOURCES)
	$(CXX) $(CPPFLAGS) -MM $(SOURCES) >.d
-include .d
$(TARGET): $(OBJECTS) 
	$(CXX) $(OBJECTS) -o $@

clean:
	rm $(OBJECTS)
