#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

extern std::string ARRAY_SIZE_ATTRIBUTE;
extern std::string ARRAY_SIZE_REF_ATTRIBUTE;
extern std::string NULL_TERMINATED_ATTRIBUTE;

class Attribute
{
public:
	virtual std::string getAttributeType() const = 0;
};

class ArraySizeAttribute : public Attribute
{
	unsigned int size;
public:

	ArraySizeAttribute(unsigned int size) :
	  size(size)
	{
	}

	virtual std::string getAttributeType() const
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

	virtual std::string getAttributeType() const
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
	virtual std::string getAttributeType() const
	{
		return NULL_TERMINATED_ATTRIBUTE;
	}
};

class FormatMember
{
	std::wstring name;
	std::wstring tname;
	std::vector< std::shared_ptr<Attribute> > attrs;

public:
	FormatMember()
	{
	}

	FormatMember(std::wstring name, 
		std::wstring tname, 
		std::vector< std::shared_ptr<Attribute> > attrs) : 
	name(name), tname(tname), attrs(attrs)
	{
	}

	std::wstring GetName() const
	{
		return name;
	}

	std::wstring GetTypeName() const
	{
		return tname;
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
	std::vector<FormatMember> members;
	std::wstring name;

public:
	FormatDesc()
	{
	}

	FormatDesc(std::wstring name) : 
	name(name)
	{
	}

	void Add(FormatMember m)
	{
		members.push_back(m);
	}

	std::wstring GetName() const
	{
		return name;
	}

	void ForeachMember(std::function<void(FormatMember&)> action)
	{
		for (size_t i=0; i<members.size(); i++)
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
