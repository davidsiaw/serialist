
#ifndef SERIALIST_HPP
#define SERIALIST_HPP

/*
	WARNING: This file is generated using ruco. Please modify the .ruco file if you wish to change anything
	https://github.com/davidsiaw/ruco
*/

#include <string>
#include <memory>
#include <vector>

namespace Serialist
{

enum ElementType
{
	FORMAT_ELEMENT,
	SUBSET_ELEMENT
};

class Element 
{
public:
	unsigned _line, _col;
	virtual ElementType get_element_type() const = 0;
};
typedef std::shared_ptr<Element> ElementPtr;
typedef std::vector<ElementPtr> ElementArray;

class Serialist 
{
public:
	unsigned _line, _col;
	ElementArray elements;
};
typedef std::shared_ptr<Serialist> SerialistPtr;
typedef std::vector<SerialistPtr> SerialistArray;

class NumberLit 
{
public:
	unsigned _line, _col;
	std::wstring content;
};
typedef std::shared_ptr<NumberLit> NumberLitPtr;
typedef std::vector<NumberLitPtr> NumberLitArray;

enum PrimaryType
{
	SIMPLELITERAL_PRIMARY,
	STRINGLITERAL_PRIMARY,
	ARRAYLITERAL_PRIMARY,
	MEMBERNAME_PRIMARY,
	BRACKETEDEXPRESSION_PRIMARY
};

class Primary 
{
public:
	unsigned _line, _col;
	virtual PrimaryType get_primary_type() const = 0;
};
typedef std::shared_ptr<Primary> PrimaryPtr;
typedef std::vector<PrimaryPtr> PrimaryArray;

enum SimpleLiteralType
{
	NUMBERLITERAL_SIMPLELITERAL,
	HEXLITERAL_SIMPLELITERAL,
	CHARLITERAL_SIMPLELITERAL
};

class SimpleLiteral : public Primary
{
public:
	unsigned _line, _col;
	virtual SimpleLiteralType get_simpleliteral_type() const = 0;
	virtual PrimaryType get_primary_type() const
	{
		return SIMPLELITERAL_PRIMARY;
	}

};
typedef std::shared_ptr<SimpleLiteral> SimpleLiteralPtr;
typedef std::vector<SimpleLiteralPtr> SimpleLiteralArray;

class NumberLiteral : public SimpleLiteral
{
public:
	unsigned _line, _col;
	NumberLitPtr numberlit;
	virtual SimpleLiteralType get_simpleliteral_type() const
	{
		return NUMBERLITERAL_SIMPLELITERAL;
	}

};
typedef std::shared_ptr<NumberLiteral> NumberLiteralPtr;
typedef std::vector<NumberLiteralPtr> NumberLiteralArray;

class HexLit 
{
public:
	unsigned _line, _col;
	std::wstring content;
};
typedef std::shared_ptr<HexLit> HexLitPtr;
typedef std::vector<HexLitPtr> HexLitArray;

class HexLiteral : public SimpleLiteral
{
public:
	unsigned _line, _col;
	HexLitPtr hexlit;
	virtual SimpleLiteralType get_simpleliteral_type() const
	{
		return HEXLITERAL_SIMPLELITERAL;
	}

};
typedef std::shared_ptr<HexLiteral> HexLiteralPtr;
typedef std::vector<HexLiteralPtr> HexLiteralArray;

class MemberIdentifier 
{
public:
	unsigned _line, _col;
	std::wstring content;
};
typedef std::shared_ptr<MemberIdentifier> MemberIdentifierPtr;
typedef std::vector<MemberIdentifierPtr> MemberIdentifierArray;

class MemberName : public Primary
{
public:
	unsigned _line, _col;
	MemberIdentifierPtr memberidentifier;
	virtual PrimaryType get_primary_type() const
	{
		return MEMBERNAME_PRIMARY;
	}

};
typedef std::shared_ptr<MemberName> MemberNamePtr;
typedef std::vector<MemberNamePtr> MemberNameArray;

class CharLit 
{
public:
	unsigned _line, _col;
	std::wstring content;
};
typedef std::shared_ptr<CharLit> CharLitPtr;
typedef std::vector<CharLitPtr> CharLitArray;

class CharLiteral : public SimpleLiteral
{
public:
	unsigned _line, _col;
	CharLitPtr charlit;
	virtual SimpleLiteralType get_simpleliteral_type() const
	{
		return CHARLITERAL_SIMPLELITERAL;
	}

};
typedef std::shared_ptr<CharLiteral> CharLiteralPtr;
typedef std::vector<CharLiteralPtr> CharLiteralArray;

class StringLit 
{
public:
	unsigned _line, _col;
	std::wstring content;
};
typedef std::shared_ptr<StringLit> StringLitPtr;
typedef std::vector<StringLitPtr> StringLitArray;

class StringLiteral : public Primary
{
public:
	unsigned _line, _col;
	StringLitPtr stringlit;
	virtual PrimaryType get_primary_type() const
	{
		return STRINGLITERAL_PRIMARY;
	}

};
typedef std::shared_ptr<StringLiteral> StringLiteralPtr;
typedef std::vector<StringLiteralPtr> StringLiteralArray;

enum InvocationType
{
	FUNCTIONINVOKE_INVOCATION
};

class Invocation 
{
public:
	unsigned _line, _col;
	virtual InvocationType get_invocation_type() const = 0;
};
typedef std::shared_ptr<Invocation> InvocationPtr;
typedef std::vector<InvocationPtr> InvocationArray;

class Unary 
{
public:
	unsigned _line, _col;
	PrimaryPtr primary;
	InvocationArray invocations;
};
typedef std::shared_ptr<Unary> UnaryPtr;
typedef std::vector<UnaryPtr> UnaryArray;

class Expression 
{
public:
	unsigned _line, _col;
	UnaryPtr unary;
};
typedef std::shared_ptr<Expression> ExpressionPtr;
typedef std::vector<ExpressionPtr> ExpressionArray;

class ArrayLiteral : public Primary
{
public:
	unsigned _line, _col;
	ExpressionArray expressions;
	virtual PrimaryType get_primary_type() const
	{
		return ARRAYLITERAL_PRIMARY;
	}

};
typedef std::shared_ptr<ArrayLiteral> ArrayLiteralPtr;
typedef std::vector<ArrayLiteralPtr> ArrayLiteralArray;

class BracketedExpression : public Primary
{
public:
	unsigned _line, _col;
	ExpressionPtr expression;
	virtual PrimaryType get_primary_type() const
	{
		return BRACKETEDEXPRESSION_PRIMARY;
	}

};
typedef std::shared_ptr<BracketedExpression> BracketedExpressionPtr;
typedef std::vector<BracketedExpressionPtr> BracketedExpressionArray;

class FunctionInvoke : public Invocation
{
public:
	unsigned _line, _col;
	ExpressionArray expressions;
	virtual InvocationType get_invocation_type() const
	{
		return FUNCTIONINVOKE_INVOCATION;
	}

};
typedef std::shared_ptr<FunctionInvoke> FunctionInvokePtr;
typedef std::vector<FunctionInvokePtr> FunctionInvokeArray;

class AttributeParams 
{
public:
	unsigned _line, _col;
	ExpressionArray expressions;
};
typedef std::shared_ptr<AttributeParams> AttributeParamsPtr;
typedef std::vector<AttributeParamsPtr> AttributeParamsArray;

class TypeIdentifier 
{
public:
	unsigned _line, _col;
	std::wstring content;
};
typedef std::shared_ptr<TypeIdentifier> TypeIdentifierPtr;
typedef std::vector<TypeIdentifierPtr> TypeIdentifierArray;

class Attribute 
{
public:
	unsigned _line, _col;
	TypeIdentifierPtr typeidentifier;
	AttributeParamsArray attributeparams;
};
typedef std::shared_ptr<Attribute> AttributePtr;
typedef std::vector<AttributePtr> AttributeArray;

class Statement 
{
public:
	unsigned _line, _col;
	AttributeArray attributes;
	TypeIdentifierPtr typeidentifier;
	MemberIdentifierPtr memberidentifier;
};
typedef std::shared_ptr<Statement> StatementPtr;
typedef std::vector<StatementPtr> StatementArray;

class Format : public Element
{
public:
	unsigned _line, _col;
	AttributeArray attributes;
	TypeIdentifierPtr typeidentifier;
	StatementArray statements;
	virtual ElementType get_element_type() const
	{
		return FORMAT_ELEMENT;
	}

};
typedef std::shared_ptr<Format> FormatPtr;
typedef std::vector<FormatPtr> FormatArray;

class SubsetRange 
{
public:
	unsigned _line, _col;
	SimpleLiteralArray simpleliterals;
};
typedef std::shared_ptr<SubsetRange> SubsetRangePtr;
typedef std::vector<SubsetRangePtr> SubsetRangeArray;

class Subset : public Element
{
public:
	unsigned _line, _col;
	TypeIdentifierArray typeidentifiers;
	SubsetRangePtr subsetrange;
	virtual ElementType get_element_type() const
	{
		return SUBSET_ELEMENT;
	}

};
typedef std::shared_ptr<Subset> SubsetPtr;
typedef std::vector<SubsetPtr> SubsetArray;


}

#endif // SERIALIST_HPP

