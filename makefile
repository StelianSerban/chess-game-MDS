CXX      = g++
CXXFLAGS = -std=c++17 -Wall
LIBS     = -lsfml-graphics -lsfml-window -lsfml-system
GTEST    = -lgtest -lgtest_main -pthread
TARGET   = app
TEST_TARGET = run_tests
SRCS = main.cpp Board.cpp MoveGen.cpp GameLogic.cpp Bot.cpp
TEST_SRCS = tests/test_board.cpp tests/test_movegen.cpp tests/test_gamelogic.cpp \
            Board.cpp MoveGen.cpp GameLogic.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_TARGET) $(GTEST)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)