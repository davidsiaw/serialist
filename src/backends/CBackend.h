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

	std::wstring GenErrorOutputParam()
	{
		return L"int* out_error";
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

	std::wstring GetMemberContainingArrayElementCount(const FormatMember& member)
	{
		std::wstring str = member.GetMemberContainingArraySize();
		if (str.length() == 0)
		{
			return L"COUNT_" + member.GetName();
		}
		return str;
	}

	std::wstring GetMemberContainingArrayCapacity(const FormatMember& member)
	{
		return L"CAPACITY_" + member.GetName();
	}

	std::wstring GenerateCTypeForAllocation(const FormatMember& member)
	{
		std::wstringstream ss;

		ss << CTypeName(member.GetTypeName());

		if (!member.HasBasicType())
		{
			ss << "*";
		}

		return ss.str();
	}

	std::wstring GenerateCTypeForDeclaration(const FormatMember& member)
	{
		std::wstringstream ss;

		ss << GenerateCTypeForAllocation(member);

		if (member.IsArray())
		{
			ss << "*";
		}

		return ss.str();
	}

	std::wstring padTo(const size_t num, const wchar_t paddingChar = L' ')
	{
		std::wstring str;
		str.insert(0, num - str.size(), paddingChar);
		return str;
	}

	void GenerateStructPrototype()
	{
		std::wcout << "/**" << std::endl;
		std::wcout << " * Opaque struct prototypes generated by serialist" << std::endl;
		std::wcout << " **/" << std::endl;

		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;
			std::wcout << "typedef struct _tag_" << kvpair.first << " " << kvpair.first << ";" << std::endl;
		}

		std::wcout << std::endl;
	}

	void GenerateStructDefinition()
	{
		std::wstring sizetypename = L"size_t";

		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;
			std::wcout << "struct _tag_" << kvpair.first << " {" << std::endl;

			int columnSize = 0;
			desc.ForeachMember([&](FormatMember& member)
			{
				auto size = GenerateCTypeForDeclaration(member).length();

				if (member.IsArray() && size < sizetypename.length())
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
				auto typeName = GenerateCTypeForDeclaration(member);

				std::wcout << "\t" << typeName << padTo(columnSize - typeName.length()) << member.GetName() << ";" << std::endl;

				if (member.IsLengthOnlyKnownAtRuntime())
				{
					if (!member.HasMemberContainingArraySize())
					{
						std::wcout << "\t" << sizetypename << padTo(columnSize - sizetypename.length()) << GetMemberContainingArrayElementCount(member) << ";" << std::endl;
					}
					std::wcout << "\t" << sizetypename << padTo(columnSize - sizetypename.length()) << GetMemberContainingArrayCapacity(member) << ";" << std::endl;
				}
			});

			std::wcout << "};" << std::endl << std::endl;
		}
	}

	std::wstring GenerateCreatePrototypeSignature(std::wstring formatName)
	{
		std::wstringstream ss;
		ss << formatName << "* Create" << formatName << "()";
		return ss.str();
	}


	void GenerateDeleteDefinitionForMember(FormatMember member, int indent = 0)
	{
		if (member.IsArray())
		{
			std::wstringstream initialSize;
			if (!member.IsLengthOnlyKnownAtRuntime())
			{
				initialSize << member.GetFixedArraySize();
			}
			else
			{
				initialSize << "object->" << GetMemberContainingArrayElementCount(member);
			}

			if (!member.HasBasicType())
			{
				std::wcout << padTo(indent, '\t') << "for (i=0; i<" << initialSize.str() << "; i++)" << std::endl;
				std::wcout << padTo(indent, '\t') << "{" << std::endl;
				std::wcout << padTo(indent+1, '\t') << "Delete" << member.GetTypeName() << "(&object->" << member.GetName() << "[i]);" << std::endl;
				std::wcout << padTo(indent, '\t') << "}" << std::endl;
			}

			std::wcout << padTo(indent, '\t') << "free(object->" << member.GetName() << ");" << std::endl;

		}
		else if (!member.HasBasicType())
		{
			std::wcout << padTo(indent, '\t') << "free(object->" << member.GetName() << ");" << std::endl;
		}
	}

	void GenerateErrorCaseDeallocator(std::vector<FormatMember>& membersInitialized, FormatMember& member, int indent = 0, bool arrayMember = false)
	{
		std::wcout << padTo(indent, '\t') << "if (object->" << member.GetName();
		if (arrayMember)
		{
			std::wcout << "[i]";
		}
		std::wcout << " == NULL)" << std::endl;
		std::wcout << padTo(indent, '\t') << "{" << std::endl;
		std::wcout << padTo(indent+1, '\t') << "/* Allocation of " << member.GetName() << " fails, deallocate allocations so far and return NULL */" << std::endl;

		if (arrayMember)
		{
			membersInitialized.push_back(member);
		}

		for (int i=membersInitialized.size()-1; i>=0 ;i--)
		{
			GenerateDeleteDefinitionForMember(membersInitialized[i], indent+1);
		}

		std::wcout << padTo(indent+1, '\t') << "free(object);" << std::endl;
		std::wcout << padTo(indent+1, '\t') << "return NULL;" << std::endl;
		std::wcout << padTo(indent, '\t') << "}" << std::endl;
	
	}

	void GenerateCreateFunction(FormatDesc& desc, bool prototype)
	{
		std::wcout << GenerateCreatePrototypeSignature(desc.GetName());
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl << "{" << std::endl;
			if (!desc.IsTrivial())
			{
				std::wcout << "\tsize_t i = 0;" << std::endl;
			}
			std::wcout << "\t" << desc.GetName() << "* object = calloc(1, sizeof(" << desc.GetName() << "));" << std::endl;

			std::vector<FormatMember> membersInitialized;
			desc.ForeachMember([&](FormatMember& member)
			{
				if (member.IsArray())
				{
					int initialSize = 0;
					if (!member.IsLengthOnlyKnownAtRuntime())
					{
						initialSize = member.GetFixedArraySize();
					}

					if (initialSize != 0)
					{
						std::wcout << "\tobject->" << member.GetName() << " = calloc(" << initialSize << ", sizeof(" << GenerateCTypeForAllocation(member) << ")" << ");" << std::endl;

						GenerateErrorCaseDeallocator(membersInitialized, member, 1);

						if (!member.HasBasicType())
						{
							std::wcout << "\tfor (i=0; i<" << initialSize << "; i++)" << std::endl;
							std::wcout << "\t{" << std::endl;
							std::wcout << "\t\tobject->" << member.GetName() << "[i] = Create" << member.GetTypeName() << "();" << std::endl;

							GenerateErrorCaseDeallocator(membersInitialized, member, 2, true);

							std::wcout << "\t}" << std::endl;
						}
					}
				}
				else if (!member.HasBasicType())
				{
					std::wcout << "\tobject->" << member.GetName() << " = Create" << member.GetTypeName() << "();" << std::endl;

					GenerateErrorCaseDeallocator(membersInitialized, member, 1);
				}
				membersInitialized.push_back(member);
			});

			std::wcout << "\treturn object;" << std::endl;
			std::wcout << "}" << std::endl;
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
			GenerateCreateFunction(desc, true);
			std::wcout << std::endl;
		}
	}

	void GenerateCreateDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;
			GenerateCreateFunction(desc, false);
			std::wcout << std::endl;
		}
	}

	void GenerateDeleteFunction(FormatDesc& desc, bool prototype)
	{
		std::wcout << "void Delete" << desc.GetName() << "(" << desc.GetName() << "** ref_object)";
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;
			if (!desc.IsTrivial())
			{
				std::wcout << "\tsize_t i = 0;" << std::endl;
			}
			std::wcout << "\t" << desc.GetName() << "* object;" << std::endl;

			std::wcout << "\tif (ref_object == NULL || *ref_object == NULL) { return; }" << std::endl;
			std::wcout << "\tobject = *ref_object;" << std::endl;

			desc.ForeachMemberBackwards([&](FormatMember& member)
			{
				GenerateDeleteDefinitionForMember(member, 1);
			});

			std::wcout << "\tfree(object);" << std::endl;
			std::wcout << "\t*ref_object = NULL;" << std::endl;

			std::wcout << "}" << std::endl;
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
			GenerateDeleteFunction(desc, true);

			std::wcout << std::endl;
		}
	}

	void GenerateDeleteDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;
			GenerateDeleteFunction(desc, false);

			std::wcout << std::endl;
		}
	}


	void GenerateWriteFunction(FormatDesc& desc, bool prototype)
	{
		std::wcout << "void Write" << desc.GetName() << "(" << desc.GetName() << "* object, unsigned char** out_bytes, size_t* out_length, " << GenErrorOutputParam() << ")";
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			std::wcout << "\tsize_t pos = 0;" << std::endl;
			std::wcout << "\tsize_t datasize = 0;" << std::endl;
			std::wcout << "\tsize_t length = 0;" << std::endl;
			std::wcout << "\tunsigned char* bytes = NULL;" << std::endl;
			std::wcout << "\tunsigned char* temp_bytes = NULL;" << std::endl;
			std::wcout << "\t*out_error = NO_ERROR;" << std::endl;
			std::wcout << "\tsize_t i = 0;" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{
				std::wcout << "\t/* Write " << member.GetName() << " */" << std::endl;

				if (member.IsArray())
				{
					std::wstringstream size_expression;

					if (member.HasAttribute(ARRAY_SIZE_ATTRIBUTE))
					{
						size_expression << member.GetFixedArraySize();
					}
					else
					{
						size_expression << "object->" << GetMemberContainingArrayElementCount(member);
					}

					std::wcout << "\tfor (i = 0; i < " << size_expression.str() << "; i++)" << std::endl;
					std::wcout << "\t{" << std::endl;

					if (member.HasBasicType())
					{
						std::wcout << "\t\tdatasize = sizeof(" << GenerateCTypeForAllocation(member) << ");" << std::endl;
						std::wcout << "\t\tlength += datasize;" << std::endl;
						std::wcout << "\t\tbytes = realloc(bytes, length);" << std::endl;
						std::wcout << "\t\tmemcpy(bytes + pos, &object->" << member.GetName() << "[i], datasize);" << std::endl;
						std::wcout << "\t\tpos = length;" << std::endl;
					}
					else
					{
						std::wcout << "\t\tWrite" << member.GetTypeName() << "(object->" << member.GetName() << "[i], &temp_bytes, &datasize, out_error);" << std::endl;
						std::wcout << "\t\tlength += datasize;" << std::endl;
						std::wcout << "\t\tbytes = realloc(bytes, length);" << std::endl;
						std::wcout << "\t\tmemcpy(bytes + pos, temp_bytes, datasize);" << std::endl;
						std::wcout << "\t\tpos = length;" << std::endl;
						std::wcout << "\t\tfree(temp_bytes);" << std::endl;
						std::wcout << "\t\ttemp_bytes = NULL;" << std::endl;
					}

					std::wcout << "\t}" << std::endl;

					if (member.HasBasicType() && member.HasAttribute(NULL_TERMINATED_ATTRIBUTE))
					{
						std::wcout << "\tdatasize = sizeof(" << GenerateCTypeForAllocation(member) << ");" << std::endl;
						std::wcout << "\tlength += datasize;" << std::endl;
						std::wcout << "\tbytes = realloc(bytes, length);" << std::endl;
						std::wcout << "\tmemset(bytes + pos, 0, datasize);" << std::endl;
						std::wcout << "\tpos = length;" << std::endl;
					}
				}
				else
				{
					if (member.HasBasicType())
					{
						std::wcout << "\tdatasize = sizeof(" << GenerateCTypeForAllocation(member) << ");" << std::endl;
						std::wcout << "\tlength += datasize;" << std::endl;
						std::wcout << "\tbytes = realloc(bytes, length);" << std::endl;
						std::wcout << "\tmemcpy(bytes + pos, &object->" << member.GetName() << ", datasize);" << std::endl;
						std::wcout << "\tpos = length;" << std::endl;
					}
					else
					{
						std::wcout << "\tWrite" << member.GetTypeName() << "(object->" << member.GetName() << ", &temp_bytes, &datasize, out_error);" << std::endl;
						std::wcout << "\tlength += datasize;" << std::endl;
						std::wcout << "\tbytes = realloc(bytes, length);" << std::endl;
						std::wcout << "\tmemcpy(bytes + pos, temp_bytes, datasize);" << std::endl;
						std::wcout << "\tpos = length;" << std::endl;
						std::wcout << "\tfree(temp_bytes);" << std::endl;
						std::wcout << "\ttemp_bytes = NULL;" << std::endl;
					}
				}
			});

			std::wcout << "\t*out_length = length;" << std::endl;
			std::wcout << "\t*out_bytes = bytes;" << std::endl;

			std::wcout << "}" << std::endl;
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
			GenerateWriteFunction(desc, true);
			
			std::wcout << std::endl;
		}
	}

	void GenerateWriteDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;
			GenerateWriteFunction(desc, false);

			std::wcout << std::endl;
		}
	}

	void GenerateReadFunction(FormatDesc& desc, bool prototype)
	{
		std::wcout << desc.GetName() << "* Read" << desc.GetName() << "(" << "unsigned char* bytes, size_t length, size_t start, size_t* out_end, " << GenErrorOutputParam() << ")";
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			std::wcout << "\t" << desc.GetName() << "* object = Create" << desc.GetName() << "();" << std::endl;
			std::wcout << "\tsize_t pos = start;" << std::endl;
			std::wcout << "\tsize_t end;" << std::endl;
			std::wcout << "\tsize_t i = 0;" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{
				if (member.HasAttribute(NULL_TERMINATED_ATTRIBUTE) || 
					(member.IsArray() && !member.HasBasicType()) )
				{
					std::wcout << "\t" << GenerateCTypeForAllocation(member) << " temp_" << member.GetName() << ";" << std::endl;
				}
			});

			std::wcout << "\t/* End declarations */" << std::endl;


			std::wcout << "\tif (object == NULL)" << std::endl;
			std::wcout << "\t{" << std::endl;
			std::wcout << "\t\t*out_error = ALLOC_FAILED;" << std::endl;
			std::wcout << "\t\treturn NULL;" << std::endl;
			std::wcout << "\t}" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{
				std::wcout << "\t/* Read " << member.GetName() << " */" << std::endl;

				if (member.IsArray())
				{
					if (member.HasBasicType())
					{
						if (member.HasAttribute(ARRAY_SIZE_ATTRIBUTE))
						{
							std::wcout << "\tmemcpy(object->" << member.GetName() << ", bytes + pos, sizeof(" << GenerateCTypeForAllocation(member) << ") * " << member.GetFixedArraySize() << ");" << std::endl;
							std::wcout << "\tpos += sizeof(" << GenerateCTypeForAllocation(member) << ") * " << member.GetFixedArraySize() << ";" << std::endl;
						}
						else if (member.HasAttribute(ARRAY_SIZE_REF_ATTRIBUTE))
						{
							std::wcout << "\tobject->" << member.GetName() << " = calloc(object->" << GetMemberContainingArrayElementCount(member) << ", sizeof(" << GenerateCTypeForAllocation(member) << "))" << ";" << std::endl;

							std::wcout << "\tif (object->" << member.GetName() << " == NULL)" << std::endl;
							std::wcout << "\t{" << std::endl;
							std::wcout << "\t\t*out_error = ALLOC_FAILED;" << std::endl;
							std::wcout << "\t\tDelete" << desc.GetName() << "(&object);" << std::endl;
							std::wcout << "\t\treturn NULL;" << std::endl;
							std::wcout << "\t}" << std::endl;

							std::wcout << "\tmemcpy(object->" << member.GetName() << ", bytes + pos, sizeof(" << GenerateCTypeForAllocation(member) << ") * object->" << GetMemberContainingArrayElementCount(member) << ");" << std::endl;
							std::wcout << "\tpos += sizeof(" << GenerateCTypeForAllocation(member) << ") * object->" << GetMemberContainingArrayElementCount(member) << ";" << std::endl;

						}
						else if (member.HasAttribute(NULL_TERMINATED_ATTRIBUTE))
						{
							std::wcout << "\tdo" << std::endl;
							std::wcout << "\t{" << std::endl;

							std::wcout << "\t\tmemcpy(&temp_" << member.GetName() << ", bytes + pos, sizeof(" << GenerateCTypeForAllocation(member) << "));" << std::endl;
							std::wcout << "\t\t" << "AddTo" << desc.GetName() << "_" << member.GetName() << "(object, temp_" << member.GetName() << ", out_error);" << std::endl;
							std::wcout << "\t\tpos += sizeof(" << GenerateCTypeForAllocation(member) << ");" << std::endl;
							std::wcout << "\t\tif (out_error != NO_ERROR)" << std::endl;
							std::wcout << "\t\t{" << std::endl;
							std::wcout << "\t\t\tDelete" << desc.GetName() << "(&object);" << std::endl;
							std::wcout << "\t\t\treturn NULL;" << std::endl;
							std::wcout << "\t\t}" << std::endl;

							std::wcout << "\t}" << std::endl;
							std::wcout << "\twhile (temp_" << member.GetName() << ");" << std::endl;
						}
					}
					else
					{
						std::wstringstream size_expression;

						if (member.HasAttribute(ARRAY_SIZE_ATTRIBUTE))
						{
							size_expression << member.GetFixedArraySize();
						}
						else if (member.HasAttribute(ARRAY_SIZE_REF_ATTRIBUTE))
						{
							size_expression << "object->" << GetMemberContainingArrayElementCount(member);
							std::wcout << "\tobject->" << member.GetName() << " = calloc(" << size_expression.str() << ", sizeof(" << GenerateCTypeForAllocation(member) << "))" << ";" << std::endl;

							std::wcout << "\tif (object->" << member.GetName() << " == NULL)" << std::endl;
							std::wcout << "\t{" << std::endl;
							std::wcout << "\t\t*out_error = ALLOC_FAILED;" << std::endl;
							std::wcout << "\t\tDelete" << desc.GetName() << "(&object);" << std::endl;
							std::wcout << "\t\treturn NULL;" << std::endl;
							std::wcout << "\t}" << std::endl;
						}

						std::wcout << "\tfor(i = 0; i < " << size_expression.str() << "; i++)" << std::endl;
						std::wcout << "\t{" << std::endl;

						std::wcout << "\t\ttemp_" << member.GetName() << " = Read" << member.GetTypeName() << "(bytes, length, pos, &end, out_error);" << std::endl;

						std::wcout << "\t\tif (out_error != NO_ERROR)" << std::endl;
						std::wcout << "\t\t{" << std::endl;
						std::wcout << "\t\t\tDelete" << desc.GetName() << "(&object);" << std::endl;
						std::wcout << "\t\t\treturn NULL;" << std::endl;
						std::wcout << "\t\t}" << std::endl;

						std::wcout << "\t\tpos = end;" << std::endl;

						std::wcout << "\t\tobject->" << member.GetName() << "[i] = temp_" << member.GetName() << ";" << std::endl;

						std::wcout << "\t}" << std::endl;

					}

				}
				else
				{
					if (member.HasBasicType())
					{
						std::wcout << "\tmemcpy(&object->" << member.GetName() << ", bytes + pos, sizeof(" << GenerateCTypeForAllocation(member) << "));" << std::endl;
						std::wcout << "\tpos += sizeof(" << GenerateCTypeForAllocation(member) << ");" << std::endl;
					}
					else
					{
						std::wcout << "\tobject->" << member.GetName() << " = Read" << member.GetTypeName() << "(bytes, length, pos, &end, out_error);" << std::endl;

						std::wcout << "\tif (out_error != NO_ERROR)" << std::endl;
						std::wcout << "\t{" << std::endl;
						std::wcout << "\t\tDelete" << desc.GetName() << "(&object);" << std::endl;
						std::wcout << "\t\treturn NULL;" << std::endl;
						std::wcout << "\t}" << std::endl;

						std::wcout << "\tpos = end;" << std::endl;
					}
				}

			});

			std::wcout << "\t*out_end = pos;" << std::endl;
			std::wcout << "\treturn object;" << std::endl;

			std::wcout << "}" << std::endl;
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
			GenerateReadFunction(desc, true);

			std::wcout << std::endl;
		}
	}

	void GenerateReadDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			GenerateReadFunction(desc, false);

			std::wcout << std::endl;
		}
	}

	void GenerateObjectPresenceCheck(int indent, bool basicType, bool returnRequired = true)
	{
		std::wstring defaultReturn = L"NULL";
		if (basicType)
		{
			defaultReturn = L"0";
		}

		std::wcout << padTo(indent, '\t') << "if (out_error != NULL) { *out_error = NO_ERROR; }" << std::endl;

		std::wcout << padTo(indent, '\t') << "if (object == NULL)" << std::endl;
		std::wcout << padTo(indent, '\t') << "{" << std::endl;

		std::wcout << padTo(indent+1, '\t') << "if (out_error != NULL) { *out_error = NULL_OBJECT; }" << std::endl;

		if (returnRequired)
		{
			std::wcout << padTo(indent+1, '\t') << "return " << defaultReturn << ";" << std::endl;
		}
		else
		{
			std::wcout << padTo(indent+1, '\t') << "return;" << std::endl;
		}

		std::wcout << padTo(indent, '\t') << "}" << std::endl;
	}

	void GenerateArrayIndexCheck(int indent, const FormatMember& member, bool returnRequired = true)
	{
		std::wstring defaultReturn = L"NULL";
		if (member.HasBasicType())
		{
			defaultReturn = L"0";
		}

		if (member.IsLengthOnlyKnownAtRuntime())
		{
			std::wcout << padTo(indent, '\t') << "if (index >= object->" << GetMemberContainingArrayElementCount(member) << ")" << std::endl;
		}
		else
		{
			std::wcout << padTo(indent, '\t') << "if (index >= " << member.GetFixedArraySize() << ")" << std::endl;
		}
		std::wcout << padTo(indent, '\t') << "{" << std::endl;
		std::wcout << padTo(indent+1, '\t') << "if (out_error != NULL) { *out_error = INDEX_OUT_OF_BOUNDS; }" << std::endl;

		if (returnRequired)
		{
			std::wcout << padTo(indent+1, '\t') << "return " << defaultReturn << ";" << std::endl;
		}
		else
		{
			std::wcout << padTo(indent+1, '\t') << "return;" << std::endl;
		}

		std::wcout << padTo(indent, '\t') << "}" << std::endl;
	}

	void GenerateGetArrayAccessorFunction(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << GenerateCTypeForAllocation(member) << " Get" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, size_t index, " << GenErrorOutputParam() << ")";
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			GenerateObjectPresenceCheck(1, member.HasBasicType());
			GenerateArrayIndexCheck(1, member);

			std::wcout << "\treturn object->" << member.GetName() << "[index];" << std::endl;

			std::wcout << "}" << std::endl;
		}
	}

	void GenerateGetArrayCountFunction(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << "size_t GetNumberOf" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, " << GenErrorOutputParam() << ")";
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			GenerateObjectPresenceCheck(1, true);

			if (member.IsLengthOnlyKnownAtRuntime())
			{
				std::wcout << "\treturn object->" << GetMemberContainingArrayElementCount(member) << ";" << std::endl;
			}
			else
			{
				std::wcout << "\treturn " << member.GetFixedArraySize() << ";" << std::endl;
			}

			std::wcout << "}" << std::endl;
		}
	}

	void GenerateGetAccessorFunction(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << GenerateCTypeForAllocation(member) << " Get" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, " << GenErrorOutputParam() << ")";
		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			GenerateObjectPresenceCheck(1, member.HasBasicType());

			std::wcout << "\treturn object->" << member.GetName() << ";" << std::endl;
			
			std::wcout << "}" << std::endl;
		}
	}

	void GenerateGetAccessorPrototype()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "/**" << std::endl;
			std::wcout << " * Get accessors for the struct " << desc.GetName() << std::endl;
			std::wcout << " **/" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{
				if (member.IsArray())
				{
					GenerateGetArrayAccessorFunction(desc, member, true);
					GenerateGetArrayCountFunction(desc, member, true);
				}
				else
				{
					GenerateGetAccessorFunction(desc, member, true);
				}
				std::wcout << std::endl;
			});

		}
	}

	void GenerateGetAccessorDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			desc.ForeachMember([&](FormatMember& member)
			{
				if (member.IsArray())
				{
					GenerateGetArrayAccessorFunction(desc, member, false);
					std::wcout << std::endl;
					GenerateGetArrayCountFunction(desc, member, false);
				}
				else
				{
					GenerateGetAccessorFunction(desc, member, false);
				}
				std::wcout << std::endl;
			});

		}
	}

	void GenerateArraySetAccessor(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << "void Set" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, size_t index, " << GenerateCTypeForAllocation(member) << " value, " << GenErrorOutputParam() << ")";

		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			GenerateObjectPresenceCheck(1, member.HasBasicType(), false);
			GenerateArrayIndexCheck(1, member, false);

			std::wcout << "\tobject->" << member.GetName() << "[index] = value;" << std::endl;

			std::wcout << "}" << std::endl;
		}

	}

	void GenerateArrayAddAccessor(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << "void AddTo" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, " << GenerateCTypeForAllocation(member) << " value, " << GenErrorOutputParam() << ")";

		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			std::wcout << "\tsize_t newCapacity = object->" << GetMemberContainingArrayElementCount(member) << ";" << std::endl;
			std::wcout << "\t" << GenerateCTypeForDeclaration(member) << " newArray;" << std::endl;

			GenerateObjectPresenceCheck(1, member.HasBasicType(), false);

			std::wcout << "\tif (newCapacity == object->" << GetMemberContainingArrayElementCount(member) << ")" << std::endl;
			std::wcout << "\t{" << std::endl;
			std::wcout << "\t\tif (newCapacity == 0)" << std::endl;
			std::wcout << "\t\t{" << std::endl;
			std::wcout << "\t\t\tnewCapacity = 2; " << std::endl;
			std::wcout << "\t\t}" << std::endl;
			std::wcout << "\t\telse" << std::endl;
			std::wcout << "\t\t{" << std::endl;
			std::wcout << "\t\t\tnewCapacity *= 2; " << std::endl;
			std::wcout << "\t\t}" << std::endl;
			std::wcout << std::endl;
			std::wcout << "\t\tnewArray = realloc(object->" << member.GetName() << ", sizeof(" << GenerateCTypeForAllocation(member) << ") * newCapacity);" << std::endl;

			std::wcout << "\t\tif (newArray == NULL)" << std::endl;
			std::wcout << "\t\t{" << std::endl;
			std::wcout << "\t\t\t*out_error = ALLOC_FAILED;" << std::endl;
			std::wcout << "\t\t\treturn;" << std::endl;
			std::wcout << "\t\t}" << std::endl;

			std::wcout << "\t\t" << "object->" << member.GetName() << " = newArray;" << std::endl;
			std::wcout << "\t\t" << "object->" << GetMemberContainingArrayCapacity(member) << " = newCapacity;" << std::endl;
			
			std::wcout << "\t}" << std::endl;
			std::wcout << std::endl;
			std::wcout << "\tobject->" << member.GetName() << "[object->" << GetMemberContainingArrayElementCount(member) << "] = value;" << std::endl;
			std::wcout << "\tobject->" << GetMemberContainingArrayElementCount(member) << "++;" << std::endl;
			std::wcout << "}" << std::endl;
		}
	}

	void GenerateArrayRemoveFromAccessor(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << "void RemoveFrom" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, size_t index, " << GenErrorOutputParam() << ")";

		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			GenerateObjectPresenceCheck(1, member.HasBasicType(), false);
			GenerateArrayIndexCheck(1, member, false);

			std::wcout << "\tif (index == object->" << GetMemberContainingArrayElementCount(member) << " - 1)" << std::endl;
			std::wcout << "\t{" << std::endl;
			if (member.HasBasicType())
			{
				std::wcout << "\t\tobject->" << member.GetName() << "[index] = 0;" << std::endl;
			}
			else
			{
				std::wcout << "\t\tobject->" << member.GetName() << "[index] = NULL;" << std::endl;
			}
			std::wcout << "\t}" << std::endl;
			std::wcout << "\telse" << std::endl;
			std::wcout << "\t{" << std::endl;
			std::wcout << "\t\tobject->" << member.GetName() << "[index] = object->" << member.GetName() << "[object->" << GetMemberContainingArrayElementCount(member) << " - 1];" << std::endl;
			std::wcout << "\t}" << std::endl;
			std::wcout << "\tobject->" << GetMemberContainingArrayElementCount(member) << "--;" << std::endl;

			std::wcout << "}" << std::endl;
		}
	}

	void GenerateMemberSetAccessor(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		std::wcout << "void Set" << desc.GetName() << "_" << member.GetName() << "(" << desc.GetName() << "* object, " << GenerateCTypeForAllocation(member) << " value, " << GenErrorOutputParam() << ")";

		if (prototype)
		{
			std::wcout << ";" << std::endl;
		}
		else
		{
			std::wcout << std::endl;
			std::wcout << "{" << std::endl;

			GenerateObjectPresenceCheck(1, member.HasBasicType(), false);

			std::wcout << "\tobject->" << member.GetName() << " = value;" << std::endl;

			std::wcout << "}" << std::endl;
		}
	}

	void GenerateSetAccessorFunctions(FormatDesc& desc, FormatMember& member, bool prototype)
	{
		if (member.IsArray())
		{
			GenerateArraySetAccessor(desc, member, prototype);
			
			if (member.IsLengthOnlyKnownAtRuntime())
			{
				GenerateArrayAddAccessor(desc, member, prototype);
				GenerateArrayRemoveFromAccessor(desc, member, prototype);
			}
		}
		else
		{
			GenerateMemberSetAccessor(desc, member, prototype);
		}
	}

	void GenerateSetAccessorPrototype()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			std::wcout << "/**" << std::endl;
			std::wcout << " * Set accessors for the struct " << desc.GetName() << std::endl;
			std::wcout << " **/" << std::endl;

			desc.ForeachMember([&](FormatMember& member)
			{
				GenerateSetAccessorFunctions(desc, member, true);
				std::wcout << std::endl;
			});

		}
	}

	void GenerateSetAccessorDefinition()
	{
		for (auto kvpair : formats)
		{
			FormatDesc& desc = kvpair.second;

			desc.ForeachMember([&](FormatMember& member)
			{
				GenerateSetAccessorFunctions(desc, member, false);
				std::wcout << std::endl;
			});
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

		std::wcout << "#ifndef _SERIALIST_DEFINES_" << std::endl;
		std::wcout << "#define _SERIALIST_DEFINES_" << std::endl;
		std::wcout << "#define NO_ERROR            0" << std::endl;
		std::wcout << "#define NULL_OBJECT         1" << std::endl;
		std::wcout << "#define ALLOC_FAILED        2" << std::endl;
		std::wcout << "#define INDEX_OUT_OF_BOUNDS 3" << std::endl;
		std::wcout << "#endif // _SERIALIST_DEFINES_" << std::endl << std::endl;

		std::wcout << "#ifdef __cplusplus" << std::endl;
		std::wcout << "extern \"C\" {" << std::endl;
		std::wcout << "#endif" << std::endl << std::endl;

		GenerateStructPrototype();
		GenerateCreatePrototype();
		GenerateDeletePrototype();
		GenerateWritePrototype();
		GenerateReadPrototype();
		GenerateGetAccessorPrototype();
		GenerateSetAccessorPrototype();

		std::wcout << "#ifdef __cplusplus" << std::endl;
		std::wcout << "}" << std::endl;
		std::wcout << "#endif" << std::endl << std::endl;

		std::wcout << "#endif // " << headerDefine << std::endl;
	}

	void GenerateSource()
	{
		std::wcout << "#include <stdlib.h>" << std::endl << std::endl;
		std::wcout << "#include <string.h>" << std::endl << std::endl;
		std::wcout << "#include \"" << name << ".h\"" << std::endl << std::endl;

		GenerateStructDefinition();
		GenerateCreateDefinition();
		GenerateDeleteDefinition();
		GenerateWriteDefinition();
		GenerateReadDefinition();
		GenerateGetAccessorDefinition();
		GenerateSetAccessorDefinition();
	}

};

#endif

