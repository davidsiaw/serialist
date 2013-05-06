# this tests that the H source produced has the right define
NAME=`../serialist/serialist h $srcdir/test.format`
echo name = $NAME
[[ $NAME == "#ifndef TEST_H"* ]] && exit 0 || exit 1