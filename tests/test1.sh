# this tests that the name is inferred from the name of the format filename
NAME=`../serialist/serialist n $srcdir/test.format`
[[ $VERBOSE == 1 ]] && echo name = $NAME
[[ $NAME == "test" ]] && exit 0 || exit 1