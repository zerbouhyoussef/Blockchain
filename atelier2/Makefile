CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall
TARGET = workshop

all: $(TARGET)
$(TARGET): main.cpp sha256.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET)
