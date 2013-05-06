TESTS = test1.sh test2.sh test3.sh test4

check_PROGRAMS = test4

test4_SOURCES = test4.cpp test.cpp
BUILT_SOURCES = test.cpp

CLEANFILES = $(BUILT_SOURCES)

test.cpp : test.format
	../serialist/serialist h test.format > test.h
	../serialist/serialist c test.format > test.cpp
	