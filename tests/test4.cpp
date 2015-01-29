#include <iostream>
#include "test.h"

int main()
{
	OtherFormat* other = CreateOtherFormat();

	SomeFormat* some = CreateSomeFormat();

	DeleteSomeFormat(&some);

	DeleteOtherFormat(&other);

	return 0;
}
