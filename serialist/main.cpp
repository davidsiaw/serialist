#include "stdafx.h"

#include "Parser.h"
#include "Scanner.h"
#include "DataStructures.h"
#include "CBackend.h"
#include "Generator.h"

int main()
{
	Scanner s(L"test.txt");
	Parser p(&s);
	p.Parse();

	Generator<CHeaderBackend> headergen;
	headergen.Generate(p.formats, std::wcout);

	Generator<CSourceBackend> sourcegen;
	sourcegen.Generate(p.formats, std::wcout);

	return EXIT_SUCCESS;
}
