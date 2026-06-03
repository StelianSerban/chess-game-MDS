CXX      = g++
CXXFLAGS = -std=c++17 -Wall
LIBS     = -lsfml-graphics -lsfml-window -lsfml-system
TARGET   = app
SRCS     = main.cpp Board.cpp MoveGen.cpp GameLogic.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)