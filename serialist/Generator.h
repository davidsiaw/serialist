#ifndef GENERATOR_H
#define GENERATOR_H

#include "DataStructures.h"

template<typename TScheme>
class Generator
{
public:
	Generator()
	{
	}

	void Generate(std::wstring name, std::vector<FormatDesc> format, std::wostream& output)
	{
		TScheme sch(name);
		sch.GenerateHeader(output);

		BOOST_FOREACH(FormatDesc fmt, format)
		{
			sch.GenerateStructOpening(fmt.GetName(), output);

			fmt.ForeachMember([&](FormatMember& member)
			{
				sch.GenerateMemberOpening(member.GetTypeName(), member.GetName(), output);

				member.ForEachAttribute([&](boost::shared_ptr<Attribute> attr)
				{
					if (attr->getAttributeType().compare(ARRAY_SIZE_ATTRIBUTE) == 0)
					{
						sch.SetMemberArraySize(
							member.GetTypeName(), 
							member.GetName(), 
							((ArraySizeAttribute*)attr.get())->GetSize(), 
							output);
					}
					else if (attr->getAttributeType().compare(ARRAY_SIZE_REF_ATTRIBUTE) == 0)
					{
						sch.SetMemberArraySizeReference(
							member.GetTypeName(), 
							member.GetName(), 
							((ArraySizeReferenceAttribute*)attr.get())->GetSizeRef(),
							output);
					}
					else
					{

					}
				});

				sch.GenerateMemberEnding(member.GetTypeName(), member.GetName(), output);
			});

			sch.GenerateStructEnding(fmt.GetName(), output);
		}
		
		sch.GenerateFooter(output);
	}
};

#endif // GENERATOR_H
