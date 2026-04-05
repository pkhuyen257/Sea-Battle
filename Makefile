CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic
TARGET ?= sea_battle.exe

SOURCES = \
	main.cpp \
	GameEngine.cpp \
	Board.cpp \
	Ship.cpp \
	Mission.cpp \
	EnemyManager.cpp \
	UIRenderer.cpp \
	SaveManager.cpp

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
