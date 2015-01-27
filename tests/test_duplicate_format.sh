#!/bin/bash
# this tests that duplicate format cannot compile
NAME=`../serialist c $srcdir/test_duplicate_format.format`
[[ $? == 1 ]] && exit 0 || exit 1
[[ $VERBOSE == 1 ]] && echo name = $NAME
