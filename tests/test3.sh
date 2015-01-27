#!/bin/bash
# this tests that the C source produced has the right include
NAME=`../serialist c $srcdir/test.format`
[[ $VERBOSE == 1 ]] && echo name = $NAME
[[ $NAME == *"#include \"test.h\""* ]] && exit 0 || exit 1