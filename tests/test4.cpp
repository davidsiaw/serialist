#include <iostream>
#include "test.h"

int main()
{

	unsigned char* bytes;
	size_t length;
	int error;

	SomeFormat* some = CreateSomeFormat();

	OtherFormat* other = GetSomeFormat_other(some, &error);

	SetOtherFormat_somenum(other, 15, &error);

	WriteSomeFormat(some, &bytes, &length, &error);


	FILE* fp = fopen("mm.bin", "wb");
	fwrite(bytes, length, 1, fp);
	fclose(fp);

	DeleteSomeFormat(&some);


	return 0;
}
