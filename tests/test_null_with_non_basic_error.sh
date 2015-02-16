#!/bin/bash
# this tests that null termination can only be used with basic formats
OUTPUT=`../serialist c $srcdir/test_null_with_non_basic_error.format`
[[ $? == 1 ]] && exit 0 || exit 1
[[ $VERBOSE == 1 ]] && echo error = $OUTPUT
