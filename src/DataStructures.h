#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>

extern std::wstring ARRAY_SIZE_ATTRIBUTE;
extern std::wstring ARRAY_SIZE_REF_ATTRIBUTE;
extern std::wstring NULL_TERMINATED_ATTRIBUTE;

class DuplicateMemberNameException {
public:
	std::wstring name;
	DuplicateMemberNameException(std::wstring name) : name(name) {}
};

class FormatNameIsBasicTypeException {
public:
	std::wstring name;
	FormatNameIsBasicTypeException(std::wstring name) : name(name) {}
};

class NullTermOnNonBasicTypeException {
public:
	std::wstring name;
	NullTermOnNonBasicTypeException(std::wstring name) : name(name) {}
};

class ReferencedMemberNotFoundException {
public:
	std::wstring soughtMemberName;
	std::wstring formatName;
	std::wstring insideMemberName;
	ReferencedMemberNotFoundException(std::wstring soughtMemberName, std::wstring formatName, std::wstring insideMemberName) : 
	soughtMemberName(soughtMemberName), formatName(formatName), insideMemberName(insideMemberName)
	{}
};

class Attribute
{
public:
	virtual std::wstring getAttributeType() const = 0;
};

class ArraySizeAttribute : public Attribute
{
	unsigned int size;
public:

	ArraySizeAttribute(unsigned int size) :
	  size(size)
	{
	}

	virtual std::wstring getAttributeType() const
	{
		return ARRAY_SIZE_ATTRIBUTE;
	}

	unsigned int GetSize() const
	{
		return size;
	}
};

class ArraySizeReferenceAttribute : public Attribute
{
	std::wstring ref;
public:

	ArraySizeReferenceAttribute(std::wstring ref) :
	  ref(ref)
	{
	}

	virtual std::wstring getAttributeType() const
	{
		return ARRAY_SIZE_REF_ATTRIBUTE;
	}

	std::wstring GetSizeRef() const
	{
		return ref;
	}
};

class NullTerminatedAttribute : public Attribute
{
public:
	virtual std::wstring getAttributeType() const
	{
		return NULL_TERMINATED_ATTRIBUTE;
	}
};

class FormatMember
{
	typedef std::vector< std::shared_ptr<Attribute> > AttrList;
	typedef std::map< std::wstring, AttrList > AttrMap;
	std::wstring name;
	std::wstring tname;
	AttrMap attributeMap;
	AttrList attrs;
	bool is_basictype;

public:
	FormatMember()
	{
	}

	FormatMember(std::wstring name, 
		std::wstring tname, 
		bool basictype,
		AttrList attrs) : 
	name(name), tname(tname), attrs(attrs), is_basictype(basictype)
	{
		for(auto attr : attrs)
		{
			if (attributeMap.find(attr->getAttributeType()) == attributeMap.end())
			{
				attributeMap[attr->getAttributeType()] = AttrList();
			}
			attributeMap[attr->getAttributeType()].push_back(attr);
			if (!is_basictype && attr->getAttributeType() == NULL_TERMINATED_ATTRIBUTE)
			{
				throw NullTermOnNonBasicTypeException(name);
			}
		}
	}

	std::wstring GetName() const
	{
		return name;
	}

	std::wstring GetTypeName() const
	{
		return tname;
	}

	bool HasBasicType() const
	{
		return is_basictype;
	}

	bool IsArray() const
	{
		return this->HasAttribute(ARRAY_SIZE_ATTRIBUTE) | 
				this->HasAttribute(ARRAY_SIZE_REF_ATTRIBUTE) |  
				this->HasAttribute(NULL_TERMINATED_ATTRIBUTE);
	}

	bool IsLengthOnlyKnownAtRuntime() const
	{
		return this->HasAttribute(ARRAY_SIZE_REF_ATTRIBUTE) | 
				this->HasAttribute(NULL_TERMINATED_ATTRIBUTE);
	}

	bool HasMemberContainingArraySize() const
	{
		return this->HasAttribute(ARRAY_SIZE_REF_ATTRIBUTE);
	}

	std::wstring GetMemberContainingArraySize() const
	{
		if (!HasMemberContainingArraySize())
		{
			return L"";
		}
		return std::dynamic_pointer_cast<ArraySizeReferenceAttribute>(
				this->GetAttributesFor(ARRAY_SIZE_REF_ATTRIBUTE)[0])->GetSizeRef();
	}

	unsigned int GetFixedArraySize() const
	{
		if (IsLengthOnlyKnownAtRuntime())
		{
			return -1;
		}
		return std::dynamic_pointer_cast<ArraySizeAttribute>(
				this->GetAttributesFor(ARRAY_SIZE_ATTRIBUTE)[0])->GetSize();
	}

	AttrList GetAttributesFor(std::wstring attrname) const
	{
		auto iter = attributeMap.find(attrname);
		if (iter == attributeMap.end())
		{
			return AttrList();
		}
		return iter->second;
	}

	bool HasAttribute(std::wstring attrname) const
	{
		if (attributeMap.find(attrname) == attributeMap.end())
		{
			return false;
		}
		return true;
	}

	void ForEachAttribute(std::function<void(std::shared_ptr<Attribute>)> func) const
	{
		for_each(attrs.begin(), attrs.end(), [&](std::shared_ptr<Attribute> attr)
		{
			func(attr);
		});
	}
};

class FormatDesc
{
	std::map<std::wstring, int> memberIndex;
	std::vector<FormatMember> members;
	std::wstring name;
	bool isTrivial;

public:
	FormatDesc()
	{
	}

	FormatDesc(std::wstring name, bool basictype) : 
	name(name), isTrivial(true)
	{
		if (basictype)
		{
			throw FormatNameIsBasicTypeException(name);
		}
	}

	void Add(FormatMember m)
	{
		if (m.HasAttribute(ARRAY_SIZE_REF_ATTRIBUTE))
		{
			auto refs = m.GetAttributesFor(ARRAY_SIZE_REF_ATTRIBUTE);
			for (std::shared_ptr<Attribute> attr : refs)
			{
				auto ref = std::dynamic_pointer_cast<ArraySizeReferenceAttribute>(attr);
				if (memberIndex.find(ref->GetSizeRef()) == memberIndex.end())
				{
					throw ReferencedMemberNotFoundException(ref->GetSizeRef(), name, m.GetName());
				}
			}
		}

		if (memberIndex.find(m.GetName()) != memberIndex.end())
		{
			throw DuplicateMemberNameException(m.GetName());
		}

		memberIndex[m.GetName()] = members.size();
		members.push_back(m);

		if (!m.HasBasicType() || m.IsLengthOnlyKnownAtRuntime())
		{
			isTrivial = false;
		}
	}

	bool IsTrivial() const
	{
		return isTrivial;
	}

	std::wstring GetName() const
	{
		return name;
	}

	void ForeachMember(std::function<void(FormatMember&)> action)
	{
		for (auto member : members)
		{
			action(member);
		}
	}

	void ForeachMemberBackwards(std::function<void(FormatMember&)> action)
	{
		for (int i=members.size()-1; i >= 0; i--)
		{
			action(members[i]);
		}
	}
};

typedef std::shared_ptr<Attribute> AttrPtr;
typedef std::shared_ptr<ArraySizeAttribute> ArrSizePtr;
typedef std::shared_ptr<ArraySizeReferenceAttribute> ArrSizeRefPtr;
typedef std::shared_ptr<NullTerminatedAttribute> NullTermPtr;

#endif // DATASTRUCTURES_H
