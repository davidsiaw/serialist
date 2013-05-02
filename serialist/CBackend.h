#ifndef CBACKEND_H
#define CBACKEND_H

#include <iostream>
#include <string>
#include <sstream>

class CHeaderBackend
{
	std::wstring arraysuffix;
	std::wstring arrayprefix;
	std::wstringstream prototypes;
	std::wstring name;

public:
	CHeaderBackend(std::wstring name);
	void GenerateHeader(std::wostream& output);
	void GenerateFooter(std::wostream& output);
	void GenerateStructOpening(const std::wstring& name, std::wostream& output);
	void GenerateStructEnding(const std::wstring& name, std::wostream& output);
	void GenerateMemberOpening(const std::wstring& tname, const std::wstring& name, std::wostream& output);
	void SetMemberArraySize(const std::wstring& tname, const std::wstring& name, unsigned int size, std::wostream& output);
	void SetMemberArraySizeReference(const std::wstring& tname, const std::wstring& name, const std::wstring& reference, std::wostream& output);
	void GenerateMemberEnding(const std::wstring& tname, const std::wstring& name, std::wostream& output);
};

class CSourceBackend
{
	std::wstring arraysuffix;
	std::wstring arraydestroyer;
	std::wstring name;

	std::wstringstream readFunctions;
	std::wstringstream writeFunctions;
	std::wstringstream destroyFunctions;

public:
	CSourceBackend(std::wstring name);
	void GenerateHeader(std::wostream& output);
	void GenerateFooter(std::wostream& output);
	void GenerateStructOpening(const std::wstring& name, std::wostream& output);
	void GenerateStructEnding(const std::wstring& name, std::wostream& output);
	void GenerateMemberOpening(const std::wstring& tname, const std::wstring& name, std::wostream& output);
	void SetMemberArraySize(const std::wstring& tname, const std::wstring& name, unsigned int size, std::wostream& output);
	void SetMemberArraySizeReference(const std::wstring& tname, const std::wstring& name, const std::wstring& reference, std::wostream& output);
	void GenerateMemberEnding(const std::wstring& tname, const std::wstring& name, std::wostream& output);
};

#endif // CBACKEND_H