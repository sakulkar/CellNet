CXX=g++
CXXFLAGS= -g -std=c++11 -Wall -c 
BIN=simulator

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -Wall -std=c++11 -g $(OBJ) -o $(BIN)

params.o: params.cpp params.h cell.h user.h
	$(CXX) $(CXXFLAGS) params.cpp

user.o: user.cpp user.h params.h cell.h
	$(CXX) $(CXXFLAGS) user.cpp

scheduler.o: scheduler.cpp scheduler.h misc.h params.h user.h cell.h
	$(CXX) $(CXXFLAGS) scheduler.cpp

cell.o: cell.cpp cell.h misc.h params.h user.h
	$(CXX) $(CXXFLAGS) cell.cpp

simu.o: simu.cpp cell.h params.h scheduler.h
	$(CXX) $(CXXFLAGS) simu.cpp

clean:
	rm -f *.o
	rm $(BIN)