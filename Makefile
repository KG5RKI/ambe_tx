
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -pthread
LDFLAGS=-g
LDLIBS=

SRCS=main.cpp userdb.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: gendb

gendb: $(OBJS)
	$(CXX) $(LDFLAGS) -o gendb $(OBJS) $(LDLIBS)

main.o: main.cpp userdb.h 
	g++ -g -c -pthread main.cpp

userdb.o: userdb.h userdb.cpp
	g++ -g -c -pthread userdb.cpp

clean:
	$(RM) $(OBJS)

