serialist
=========

Program to generate code for reading binary files described with a simple file format description language

------------
build status
============

[![Build Status](https://travis-ci.org/davidsiaw/serialist.png)](https://travis-ci.org/davidsiaw/serialist)

-------------
documentation
=============

how to build
------------

To build serialist you can either 
- use serialist.sln in MSVC 
- or with Cygwin/Linux/Mac use ./configure && make

how to use
----------

Serialist is a commandline tool that generates C code to read/write binary files.
You need a file format description and a program that uses it.

STEP 1, Make a file format:

numberlist.format

    format numberlist
    {
      uint32 numberOfNumbers
      uint32[numberOfNumbers] numbers
    }


STEP 2, Generate .h and .c files:

    # Generate header file 
    ./serialist h numberlist.format > numberlist.h
    # Generate source file
    ./serialist c numberlist.format > numberlist.c

STEP 3, Use in your C/C++ application:

    #include <iostream>
    #include "numberlist.h"
    int main()
    {
      FILE* fp = fopen("mynumbers.numbers", "r");
      numberlist* list;
      read_numberlist(fp, &list);
      for (int i=0;i<list->numberOfNumbers; i++)
      {
        std::cout << list->numbers[i] << std::endl;
      }
      return 0;
    }

TODO: more detailed

----------
motivation
==========

TODO: motivation
