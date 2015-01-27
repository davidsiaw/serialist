TESTS = test1.sh test2.sh test3.sh test_duplicate_format.sh test4 

check_PROGRAMS = test4

test4_SOURCES = test4.cpp test.c
BUILT_SOURCES = test.c

CLEANFILES = $(BUILT_SOURCES)

test.c : test.format ../serialist
	../serialist h test.format > test.h
	../serialist c test.format > test.c
	