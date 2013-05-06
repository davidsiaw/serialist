# this tests that the C source produced has the right include
NAME=`../serialist/serialist c $srcdir/test.format`
echo name = $NAME
[[ $NAME == *"#include \"test.h\""* ]] && exit 0 || exit 1