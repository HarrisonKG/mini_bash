######################################################################
# Program name: smallsh
# Author: Kristen Harrison
# Date: 6 August 2018
# Description: CS344, Final. This makefile can be run with three commands:
# 1. "make" makes the program smallsh   2. "make clean" cleans the directory   
# 3. "make memcheck" runs valgrind to test for memory leaks
######################################################################

# target: dependencies
# [tab] recipe


CXX = gcc
CXXFLAGS = -g -Wall -std=c99
VALFLAGS = --leak-check=yes --show-reachable=yes
# full valgrind flags: --tool=memcheck --leak-check=full --track-origins=yes --show-leak-kinds=all

OBJS = smallsh.o functions.o
HEADERS = functions.h 


smallsh: $(OBJS)
	$(CXX) $(OBJS) -o smallsh

smallsh.o: smallsh.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c smallsh.c

functions.o: functions.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c functions.c


clean:
	rm -rf *.o smallsh

memcheck: smallsh
	valgrind $(VALFLAGS) ./smallsh
