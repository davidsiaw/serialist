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

	AttrList GetAttributesFor (std::wstring attrname) const
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
	std::map<std::wstring, FormatMember> members;
	std::wstring name;

public:
	FormatDesc()
	{
	}

	FormatDesc(std::wstring name, bool basictype) : 
	name(name)
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
				if (members.find(ref->GetSizeRef()) == members.end())
				{
					throw ReferencedMemberNotFoundException(ref->GetSizeRef(), name, m.GetName());
				}
			}
		}

		if (members.find(m.GetName()) != members.end())
		{
			throw DuplicateMemberNameException(m.GetName());
		}
		members[m.GetName()] = m;
	}

	std::wstring GetName() const
	{
		return name;
	}

	void ForeachMember(std::function<void(FormatMember&)> action)
	{
		for (auto kvpair : members)
		{
			action(kvpair.second);
		}
	}
};

typedef std::shared_ptr<Attribute> AttrPtr;
typedef std::shared_ptr<ArraySizeAttribute> ArrSizePtr;
typedef std::shared_ptr<ArraySizeReferenceAttribute> ArrSizeRefPtr;
typedef std::shared_ptr<NullTerminatedAttribute> NullTermPtr;

#endif // DATASTRUCTURES_H
