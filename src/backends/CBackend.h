#ifndef CBACKEND_H
#define CBACKEND_H

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "../DataStructures.h"

// C Backend. Generates C89-compliant code
class CBackend
{
	std::wstring name;
	std::map<std::wstring, FormatDesc> formats;

	std::map<std::wstring, std::wstring> basicTypeMap;

	std::wstring ToUpper(std::wstring str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		return str;
	}

	std::wstring CTypeName(std::wstring originalTypeName)
	{
		auto entry = basicTypeMap.find(originalTypeName);
		if (entry == basicTypeMap.end())
		{
			return originalTypeName;
		}
		return entry->second;
	}

	std::wstring GenerateCType(std::wstring originalTypeName, bool isArray)
	{
		if (isArray)
		{
			return CTypeName(originalTypeName) + L"*";
		}
		return CTypeName(originalTypeName);
	}

	std::wstring padTo(const size_t num, const wchar_t paddingChar = L' ')
	{
		std::wstring str;
		str.insert(0, num - str.size(), paddingChar);
		return str;
	}

	void GenerateHeaderStruct()
	{
		std::wstring sizetypename = L"size_t";

		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;
			std::wcout << "typedef struct " << "{" << std::endl;

			int columnSize = 0;
			desc.ForeachMember([&](FormatMember& member)
			{
				auto size = GenerateCType(member.GetTypeName(), member.IsArray()).length();

				if (member.IsLengthOnlyKnownAtRuntime() && size < sizetypename.length())
				{
					size = sizetypename.length();
				}

				if (size > columnSize)
				{
					columnSize = size;
				}

			});
			columnSize++;

			desc.ForeachMember([&](FormatMember& member)
			{
				auto typeName = GenerateCType(member.GetTypeName(), member.IsArray());

				std::wcout << "\t" << typeName << padTo(columnSize - typeName.length()) << member.GetName() << ";" << std::endl;

				if (member.IsLengthOnlyKnownAtRuntime())
				{
					std::wcout << "\t" << sizetypename << padTo(columnSize - sizetypename.length()) << member.GetName() << "_count;" << std::endl;
				}
			});

			std::wcout << "} " << kvpair.first << ";" << std::endl << std::endl;
		}
	}

	void GenerateCreatePrototype()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "/**" << std::endl;
			std::wcout << " * Creates a " << desc.GetName() << " object and returns a pointer to it." << std::endl;
			std::wcout << " * This function will initialize all members except pointers for arrays with known sizes to zero." << std::endl;
			std::wcout << " **/" << std::endl;
			std::wcout << desc.GetName() << "* Create" << desc.GetName() << "();" << std::endl;

			std::wcout << std::endl;
		}
	}

	void GenerateCreateDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << desc.GetName() << "* Create" << desc.GetName() << "()" << std::endl;
			std::wcout << "{" << std::endl;
			std::wcout << "\t" << desc.GetName() << "* result = calloc(sizeof(" << desc.GetName() << "), 1);" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{

			});

			std::wcout << "\treturn result;" << std::endl;
			std::wcout << "}" << std::endl << std::endl;
		}
	}

	void GenerateDeletePrototype()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "/**" << std::endl;
			std::wcout << " * Deletes a " << desc.GetName() << " object and sets its pointer to NULL" << std::endl;
			std::wcout << " **/" << std::endl;
			std::wcout << "void Delete" << desc.GetName() << "(" << desc.GetName() << "** ref_victim);" << std::endl;

			std::wcout << std::endl;
		}
	}

	void GenerateDeleteDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "void Delete" << desc.GetName() << "(" << desc.GetName() << "** ref_victim)" << std::endl;
			std::wcout << "{" << std::endl;

			std::wcout << "\tif (*ref_victim == NULL) { return; }" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{

			});

			std::wcout << "\tfree(*ref_victim);" << std::endl;
			std::wcout << "\t*ref_victim = NULL;" << std::endl;

			std::wcout << "}" << std::endl << std::endl;
		}
	}

	void GenerateWritePrototype()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "/**" << std::endl;
			std::wcout << " * Allocates a byte array and serializes an existing " << desc.GetName() << " object to it." << std::endl;
			std::wcout << " **/" << std::endl;
			std::wcout << "void Write" << desc.GetName() << "(" << desc.GetName() << "* in_object, unsigned char** out_bytes, size_t* out_length);" << std::endl;

			std::wcout << std::endl;
		}
	}

	void GenerateWriteDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "void Write" << desc.GetName() << "(" << desc.GetName() << "* in_object, unsigned char** out_bytes, size_t* out_length)" << std::endl;
			std::wcout << "{" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{

			});

			std::wcout << "}" << std::endl << std::endl;


		}
	}

	void GenerateReadPrototype()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "/**" << std::endl;
			std::wcout << " * Allocates and deserializes a " << desc.GetName() << " from a byte array, and returns a pointer to it." << std::endl;
			std::wcout << " **/" << std::endl;
			std::wcout << "void Read" << desc.GetName() << "(" << desc.GetName() << "** out_object, unsigned char* in_bytes, size_t in_length);" << std::endl;

			std::wcout << std::endl;
		}
	}

	void GenerateReadDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "void Read" << desc.GetName() << "(" << desc.GetName() << "** out_object, unsigned char* in_bytes, size_t in_length)" << std::endl;
			std::wcout << "{" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{

			});

			std::wcout << "}" << std::endl << std::endl;
		}
	}


public:
	CBackend(std::wstring name, std::map<std::wstring, FormatDesc> formats) : name(name), formats(formats)
	{
		basicTypeMap[L"Uint8"] = L"unsigned char";
		basicTypeMap[L"Uint16"] = L"unsigned short";
		basicTypeMap[L"Uint32"] = L"unsigned int";
		basicTypeMap[L"Uint64"] = L"unsigned long long";
		basicTypeMap[L"Int8"] = L"signed char";
		basicTypeMap[L"Int16"] = L"short";
		basicTypeMap[L"Int32"] = L"int";
		basicTypeMap[L"Int64"] = L"long long";
		basicTypeMap[L"Float32"] = L"float";
		basicTypeMap[L"Float64"] = L"double";
	}

	void GenerateHeader()
	{
		std::wstring headerDefine = ToUpper(name) + L"_H";
		std::wcout << "#ifndef " << headerDefine << std::endl;
		std::wcout << "#define " << headerDefine << std::endl << std::endl;

		GenerateHeaderStruct();
		GenerateCreatePrototype();
		GenerateDeletePrototype();
		GenerateWritePrototype();
		GenerateReadPrototype();

		std::wcout << "#endif // " << headerDefine << std::endl;
	}

	void GenerateSource()
	{
		std::wcout << "#include <stdlib.h>" << std::endl << std::endl;
		std::wcout << "#include \"" << name << ".h\"" << std::endl << std::endl;

		GenerateCreateDefinition();
		GenerateDeleteDefinition();
		GenerateWriteDefinition();
		GenerateReadDefinition();
	}

};

#endif

