serialist
=========

Program to generate C/C++ code for reading binary files described with a simple easy-to-read file format description language.

[![Build Status](https://travis-ci.org/davidsiaw/serialist.svg)](https://travis-ci.org/davidsiaw/serialist)
[![Stories in Ready](https://badge.waffle.io/davidsiaw/serialist.svg?label=ready&title=Ready)](http://waffle.io/davidsiaw/serialist)
[![License](http://img.shields.io/badge/license-GPLv3-red.svg)](https://github.com/davidsiaw/serialist/blob/master/LICENSE)

-----------------------
a little documentation
=======================

how to install
--------------

```
gem install serialist-gem
```

how to build
------------

```
sh build.sh
```

how to use
----------

Serialist is a commandline tool that generates C code to read/write binary files.
You need a file format description and a program that uses it.

STEP 1, Make a file format:

numberlist.format

    format NumberList
    {
      Uint32 numberOfNumbers

      [ArraySize: numberOfNumbers]
      Uint32 numbers
    }


STEP 2, Generate .h and .c files:

    # Generate header file 
    serialist-gen -t CHeader numberlist.format > numberlist.h
    # Generate source file
    serialist-gen -t CSource numberlist.format > numberlist.c

STEP 3, Use in your C/C++ application:

    #include <stdio.h>
    #include "numberlist.h"
    int main()
    {
      // To read an existing file
      NumberList* list_from_file;

      FILE* fp_read = fopen("mynumbers.numbers", "rb");
      SerialistError err = ReadNumberList(fp_read, &list_from_file);
      fclose(fp_read);


      // To create and write
      NumberList* new_list;
      err = CreateNumberList(&new_list);

      FILE* fp_write = fopen("newnumbers.numbers", "wb");
      err = WriteNumberList(fp_write, new_list);
      fclose(fp_write);

      return 0;
    }

TODO: more detailed

----------
motivation
==========

While reading binary files is nothing new, and has been around since file systems even existed, 
there is surprisingly never any easy way to specify file formats. Everyone wrote a read and a
write function for every file they created, or for files they wanted to read.

Existing file format specification languages were very verbose XML-based formats XSIL, BFD and 
DFDL, and some had unusual constructs that confused first time readers such as in PyFFI.
Existing solutions such as MSGPACK and Protocol Buffers were not suitable because they were 
not made to describe already existing formats which was part of the motivation. Websites which
described file formats also used their own method of describing files in an informal manner.

My search for an easy way to read file formats beyond simply freading structs led me to write 
the FormattedReader and FormattedWriter in C# in the Blueblocks project. These two classes in 
the Blueblocks project significantly improved my ability to work with binary files, and allowed
me to make very fast changes to binary file formats and to write tools to upgrade older file 
versions.

In short, I could sit down and simply say what a file looks like inside and then read the data
from the file in to a structure for the rest of my program instead of having to worry about 
bugs in the file reading/writing code.

However the C# classes proved to be difficult to port as a lot of platforms still used C/C++
code, which do not have reflection that these classes relied on. The answer was to generate
code in the language that the program that needs to read the file is written in.

Serialist uses a very simple file format definition language that allows you to describe
almost any binary format that spans a single file, meaning the definition tells you how to
read binary files that already contain all the information you need about them (like the
sizes of each data structure is not stored in another file). I tried my best to make the
language simple enough for humans to read, yet formal enough to be used by software to read any
binary file properly, and be used as a method to share information about file formats.

Once finalized I will extend this format definition to allow formats spanning multiple files.
