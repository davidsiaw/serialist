#ifndef CPPBACKEND_H
#define CPPBACKEND_H

#include <map>
#include <string>
#include <iostream>
#include "../DataStructures.h"

// C++ Backend. Generates C++99-compliant code
class CPPBackend
{
	std::wstring name;
	std::map<std::wstring, FormatDesc> formats;
	
public:
	CPPBackend(std::wstring name, std::map<std::wstring, FormatDesc> formats) : name(name), formats(formats)
	{
	}

	void GenerateHeader()
	{
		for (auto kvpair : formats)
		{
			
		}
	}

};

#endif
