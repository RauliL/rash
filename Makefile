CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -g
RM ?= rm -f

SRCS=builtin.cpp env.cpp exec.cpp main.cpp parser.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all : rash

rash : $(OBJS)
	$(CXX) $(LDFLAGS) -o rash $(OBJS)

%.o : %.cpp rash.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	$(RM) ./rash $(OBJS)
