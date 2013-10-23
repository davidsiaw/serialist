#include "CBackend.h"

#include <boost/algorithm/string.hpp>

struct KnownTypes
{
	const wchar_t* name;
	const wchar_t* ctype;
};

static KnownTypes knownTypes[] = 
{
	{L"uint8",  L"unsigned char"},
	{L"uint16", L"unsigned short"},
	{L"uint32", L"unsigned int"},
	{L"uint64", L"unsigned long long"},
	{L"int8",  L"signed char"},
	{L"int16", L"signed short"},
	{L"int32", L"signed int"},
	{L"int64", L"signed long long"},
	{NULL, NULL}
};

std::wstring ConvertKnownType(const std::wstring& tname)
{
	for(int i=0;knownTypes[i].name;i++)
	{
		if (tname.compare(knownTypes[i].name) == 0)
		{
			return knownTypes[i].ctype;
		}
	}
	return tname;
}

bool IsKnownType(const std::wstring& tname)
{
	for(int i=0;knownTypes[i].name;i++)
	{
		if (tname.compare(knownTypes[i].name) == 0)
		{
			return true;
		}
	}
	return false;
}

std::wstring MakeSignatureForCreatorFunction(const std::wstring& verb, const std::wstring& tname)
{
	std::wstringstream str;
	str << "void " << verb << "_" << tname << "(FILE* fp, " << tname << "** resultHolder)";
	return str.str();
}

std::wstring MakeSignatureForUserFunction(const std::wstring& verb, const std::wstring& tname)
{
	std::wstringstream str;
	str << "void " << verb << "_" << tname << "(FILE* fp, " << tname << "* structure)";
	return str.str();
}

std::wstring MakeSignatureForDeleteFunction(const std::wstring& verb, const std::wstring& tname)
{
	std::wstringstream str;
	str << "void " << verb << "_" << tname << "(" << tname << "* structure)";
	return str.str();
}

CHeaderBackend::CHeaderBackend(std::wstring name)
{
	this->name = name;
}

bool IsAlphaNum(wchar_t c)
{
	return !isalnum(c);
}

std::wstring Headerize(std::wstring name)
{
	std::replace_if(name.begin(), name.end(), IsAlphaNum, L'_');
	boost::to_upper(name);
	return name + L"_H";
}

void CHeaderBackend::GenerateHeader(std::wostream& output)
{
	output << "#ifndef " << Headerize(name) << std::endl;
	output << "#define " << Headerize(name) << std::endl;
	output << "#include <stdio.h>" << std::endl;
	output << "#include <stdlib.h>" << std::endl;
	output << "#ifdef __cplusplus" << std::endl << "extern \"C\" {" << std::endl << "#endif // __cplusplus" << std::endl;
}

void CHeaderBackend::GenerateFooter(std::wostream& output)
{
	output << prototypes.str();
	output << "#ifdef __cplusplus" << std::endl << "}" << std::endl << "#endif // __cplusplus" << std::endl;
	output << "#endif // " << Headerize(name) << std::endl;
}

void CHeaderBackend::GenerateStructOpening(const std::wstring& name, std::wostream& output)
{
	output << "typedef struct" << std::endl << "{" << std::endl;
	
	prototypes << MakeSignatureForCreatorFunction(L"read", name) << ";" << std::endl;
	prototypes << MakeSignatureForUserFunction(L"write", name) << ";" << std::endl;
	prototypes << MakeSignatureForDeleteFunction(L"destroy", name) << ";" << std::endl;
}

void CHeaderBackend::GenerateStructEnding(const std::wstring& name, std::wostream& output)
{
	output << "} " << name << ";" << std::endl;
}

void CHeaderBackend::GenerateMemberOpening(const std::wstring& tname, const std::wstring& name, std::wostream& output)
{
	arraysuffix = L"";
	arrayprefix = L"";
}

void CHeaderBackend::SetMemberArraySize(const std::wstring& tname, const std::wstring& name, unsigned int size, std::wostream& output)
{
	std::wstringstream str;
	str << "[" << size << "]";
	arraysuffix = str.str();
}

void CHeaderBackend::SetMemberArraySizeReference(const std::wstring& tname, const std::wstring& name, const std::wstring& reference, std::wostream& output)
{
	arrayprefix = L"*";
}

void CHeaderBackend::SetMemberArrayNullTerminator(const std::wstring& tname, const std::wstring& name, std::wostream& output)
{
	arrayprefix = L"*";
}

void CHeaderBackend::GenerateMemberEnding(const std::wstring& tname, const std::wstring& name, std::wostream& output)
{
	if (IsKnownType(tname))
	{
		output << "\t" << ConvertKnownType(tname) << arrayprefix << " " << name << arraysuffix << ";" << std::endl;
	}
	else
	{
		output << "\t" << tname << arrayprefix << "* " << name << arraysuffix << ";" << std::endl;
	}
}


CSourceBackend::CSourceBackend(std::wstring name)
{
	this->name = name;
}

void CSourceBackend::GenerateHeader(std::wostream& output)
{
	output << "#include <stdlib.h>" << std::endl;
	output << "#include \"" << name << ".h" << "\"" << std::endl;
}

void CSourceBackend::GenerateFooter(std::wostream& output)
{
	output << writeFunctions.str();
	output << destroyFunctions.str();
}

void CSourceBackend::GenerateStructOpening(const std::wstring& name, std::wostream& output)
{
	output << std::endl << MakeSignatureForCreatorFunction(L"read", name) << std::endl 
		<< "{" << std::endl
		<< "\t" << name << "* structure = (" << name << "*)malloc(sizeof(" << name << "));" << std::endl;
	
	writeFunctions << std::endl << MakeSignatureForUserFunction(L"write", name) << std::endl 
		<< "{" << std::endl;

	destroyFunctions << std::endl << MakeSignatureForDeleteFunction(L"destroy", name) << std::endl 
		<< "{" << std::endl;

}

void CSourceBackend::GenerateStructEnding(const std::wstring& name, std::wostream& output)
{
	output << "\t*resultHolder = structure;" << std::endl;
	output << "}" << std::endl;
	
	writeFunctions << "}" << std::endl;

	destroyFunctions << "\tfree(structure);" << std::endl;
	destroyFunctions << "}" << std::endl;
}

void CSourceBackend::GenerateMemberOpening(const std::wstring& tname, const std::wstring& name, std::wostream& output)
{
	arraysuffix = L"1";
	arraydestroyer = L"";
}

void CSourceBackend::SetMemberArraySize(const std::wstring& tname, const std::wstring& name, unsigned int size, std::wostream& output)
{
	std::wstringstream str;
	str << size;
	arraysuffix = str.str();
}

void CSourceBackend::SetMemberArraySizeReference(const std::wstring& tname, const std::wstring& name, const std::wstring& reference, std::wostream& output)
{
	std::wstringstream str;
	str << "structure->" << reference;
	arraysuffix = str.str();

	if (IsKnownType(tname))
	{
		output << "\tstructure->" << name << " = (" << ConvertKnownType(tname) << "*)calloc(" << "structure->" << reference << ", " << "sizeof(" << ConvertKnownType(tname) << ")" << ");" << std::endl;
	}
	else
	{
		output << "\tstructure->" << name << " = (" << ConvertKnownType(tname) << "**)calloc(" << "structure->" << reference << ", " << "sizeof(" << ConvertKnownType(tname) << "*)" << ");" << std::endl;
	}

	std::wstringstream destroyer;
	destroyer << "\tfree(structure->" << name << ");" << std::endl;
	arraydestroyer = destroyer.str();
}

class NullTerminatorCannotBeUsedWithCustomType {};

void CSourceBackend::SetMemberArrayNullTerminator(const std::wstring& tname, const std::wstring& name, std::wostream& output)
{
	arraysuffix = L"#";
	if (IsKnownType(tname))
	{
	}
	else
	{
		throw NullTerminatorCannotBeUsedWithCustomType();
	}
}

void CSourceBackend::GenerateMemberEnding(const std::wstring& tname, const std::wstring& name, std::wostream& output)
{
	if (IsKnownType(tname))
	{
		
		if (arraysuffix == L"#")
		{
			output << "\tfor (size_t i=0;;i++)" << std::endl << "\t{" << std::endl;
			output << "\t\t" << ConvertKnownType(tname) << " temp;" << std::endl;

			output << "\t\tfread(&temp, sizeof(" << ConvertKnownType(tname) << "), 1, fp);" << std::endl;
			output << "\t\t" << "if (temp == 0)" << std::endl << "\t\t{" << std::endl;
			
			output << "\t\t\tstructure->" << name << " = (" << ConvertKnownType(tname) << "*)calloc(i+1, " << "sizeof(" << ConvertKnownType(tname) << ")" << ");" << std::endl;
			output << "\t\t\tfseek(fp, -i, SEEK_CUR);" << std::endl;
			output << "\t\t\tfread(&structure->" << name << ", sizeof(" << ConvertKnownType(tname) << "), i, fp);" << std::endl;

			output << "\t\t\t" << "break;" << std::endl;

			output << "\t\t}" << std::endl;
			output << "\t}" << std::endl;

			
			writeFunctions << "\tfor (size_t i=0;;i++)" << std::endl << "\t{" << std::endl << "\t";
			writeFunctions << "\tfwrite(&structure->" << name << "[i]" << ", sizeof(" << ConvertKnownType(tname) << "), 1, fp);" << std::endl;
			writeFunctions << "\t\tif(&structure->" << name << "[i] == 0) { break; }" << std::endl;
			writeFunctions << "\t}" << std::endl;

		}
		else 
		{
			output << "\tfread(&structure->" << name << ", sizeof(" << ConvertKnownType(tname) << "), " << arraysuffix << ", fp);" << std::endl;

			writeFunctions << "\tfwrite(&structure->" << name << ", sizeof(" << ConvertKnownType(tname) << "), " << arraysuffix << ", fp);" << std::endl;
		}
	}
	else
	{
		if (arraysuffix != L"1")
		{
			output << "\tfor (size_t i=0;i<" << arraysuffix << ";i++)" << std::endl << "\t{" << std::endl << "\t";
			output << "\tread_" << tname << "(fp, &structure->" << name << "[i]" << ");" << std::endl;
			output << "\t}" << std::endl;
			
			writeFunctions << "\tfor (size_t i=0;i<" << arraysuffix << ";i++)" << std::endl << "\t{" << std::endl << "\t";
			writeFunctions << "\twrite_" << tname << "(fp, structure->" << name << "[i]" << ");" << std::endl;
			writeFunctions << "\t}" << std::endl;

			destroyFunctions << "\tfor (size_t i=0;i<" << arraysuffix << ";i++)" << std::endl << "\t{" << std::endl << "\t";
			destroyFunctions << "\tdestroy_" << tname << "(structure->" << name << "[i]" << ");" << std::endl;
			destroyFunctions << "\t}" << std::endl;
		}
		else
		{
			output << "\tread_" << tname << "(fp, &structure->" << name << "" << ");" << std::endl;

			writeFunctions << "\twrite_" << tname << "(fp, structure->" << name << "" << ");" << std::endl;

			destroyFunctions << "\tdestroy_" << tname << "(structure->" << name << ");" << std::endl;
		}
	}
	destroyFunctions << arraydestroyer;
}
