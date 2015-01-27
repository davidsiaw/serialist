#include "stdafx.h"

#include "Parser.h"
#include "Scanner.h"
#include "DataStructures.h"

// Backends
#include "backends/CBackend.h"
#include "backends/CPPBackend.h"

enum OutputType
{
	Unknown,
	PrintName,
	CHeader,
	CSource
};

bool hasEnding (std::wstring const &fullString, std::wstring const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

std::wstring getNameWithoutExtension(std::wstring path)
{
	int firstindex = path.find_last_of(L"/\\");
	int lastindex = path.rfind(L".format");
	firstindex = firstindex == -1 ? 0 : firstindex + 1;
	lastindex = firstindex == -1 ? path.size() : lastindex;
	return path.substr(firstindex, lastindex - firstindex);
}

int main(int argc, char** argv)
{
	bool wrongUsage = false;
	OutputType type = Unknown;
	
	if (argc != 3)
	{
		wrongUsage = true;
	}
	else 
	{
		if (argv[1][0] == 'h')
		{
			type = CHeader;
		}
		else if (argv[1][0] == 'c')
		{
			type = CSource;
		}
		else if (argv[1][0] == 'n')
		{
			type = PrintName;
		}
	}

	if (wrongUsage)
	{
		std::cerr << "USAGE: serialist <h|c> <filename>.format" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::wstringstream str;
	str << argv[2];

	if (!hasEnding(str.str(), L".format"))
	{
		str << ".format";
	}

	Scanner s(str.str().c_str());
	Parser p(&s);

	try
	{
		p.Parse();

	}
	catch(DuplicateFormatNameException dfne)
	{
		std::wcerr << "Duplicate format name: " << dfne.name <<  std::endl;
		return EXIT_FAILURE;
	}
	catch(DuplicateMemberNameException dmne)
	{
		std::wcerr << "Duplicate member name: " << dmne.name <<  std::endl;
		return EXIT_FAILURE;
	}
	catch(FormatNameIsBasicTypeException fnibte)
	{
		std::wcerr << "Cannot use name: " << fnibte.name << " because it is a basic type" << std::endl;
		return EXIT_FAILURE;
	}
	catch(NullTermOnNonBasicTypeException ntonbte)
	{
		std::wcerr << "Null Termination only available on basic types: " << ntonbte.name <<  std::endl;
		return EXIT_FAILURE;
	}
	catch(ReferencedMemberNotFoundException rmnfe)
	{
		std::wcerr << "A member: " << rmnfe.soughtMemberName << " could not be found in " << rmnfe.formatName << "::" << rmnfe.insideMemberName <<  std::endl;
		return EXIT_FAILURE;
	}

	if (p.errors->count)
	{
		std::cerr << "Found " << p.errors->count << " errors. Unable to generate" << std::endl;
		return EXIT_FAILURE;
	}
	
	// Just the filename of the file
	std::wstring name = getNameWithoutExtension(str.str());

	switch(type)
	{
	case CHeader:
		{
			CBackend backend(name, p.formats);
			backend.GenerateHeader();
		}
		break;
	case CSource:
		{
			CBackend backend(name, p.formats);
			backend.GenerateSource();
		}
		break;
	case PrintName:
		{
			std::wcout << name << std::endl;
		}
		break;
	default:
		std::cerr << "Unknown file type: " << argv[1] << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
