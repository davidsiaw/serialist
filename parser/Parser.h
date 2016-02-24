

#if !defined(Serialist_COCO_PARSER_H__)
#define Serialist_COCO_PARSER_H__

#include <iostream>
#include <memory>
#include "Serialist.hpp"


#include "Scanner.h"

namespace Serialist {


class ParserException {

	int line,col;
	std::wstring message;

public:
	ParserException(int line, int col, std::wstring message) :
	line(line), col(col), message(message)
	{
	}

	int LineNumber() const
	{
		return line;
	}

	int ColumnNumber() const
	{
		return col;
	}

	std::wstring GetMessage() const
	{
		return message;
	}
};

class Errors {
public:
	int count;			// number of errors detected
	std::vector<ParserException> warnings;

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const wchar_t *s);
	void Warning(int line, int col, const wchar_t *s);
	void Warning(const wchar_t *s);
	void Exception(const wchar_t *s);

}; // Errors

class Parser {
private:
	enum {
		_EOF=0,
		_pascalcase=1,
		_camelcase=2,
		_number=3,
		_hexinteger=4,
		_string=5,
		_badString=6,
		_char=7,
		_endOfLine=8,
		_ddtSym=21,
		_optionSym=22
	};
	int maxT;

	Token *dummyToken;
	int errDist;
	int minErrDist;

	void SynErr(int n);
	void Get();
	void Expect(int n);
	bool StartOf(int s);
	void ExpectWeak(int n, int follow);
	bool WeakSeparator(int n, int syFol, int repFol);

public:
	Scanner *scanner;
	Errors  *errors;

	Token *t;			// last recognized token
	Token *la;			// lookahead token

SerialistPtr serialist;



	Parser(Scanner *scanner);
	~Parser();
	void SemErr(const wchar_t* msg);

	void Serialist();
	void Element(ElementPtr& production);
	void TypeIdentifier(TypeIdentifierPtr& production);
	void MemberIdentifier(MemberIdentifierPtr& production);
	void NumberLit(NumberLitPtr& production);
	void HexLit(HexLitPtr& production);
	void CharLit(CharLitPtr& production);
	void StringLit(StringLitPtr& production);
	void NumberLiteral(NumberLiteralPtr& production);
	void HexLiteral(HexLiteralPtr& production);
	void MemberName(MemberNamePtr& production);
	void CharLiteral(CharLiteralPtr& production);
	void StringLiteral(StringLiteralPtr& production);
	void ArrayLiteral(ArrayLiteralPtr& production);
	void Expression(ExpressionPtr& production);
	void BracketedExpression(BracketedExpressionPtr& production);
	void SimpleLiteral(SimpleLiteralPtr& production);
	void Primary(PrimaryPtr& production);
	void FunctionInvoke(FunctionInvokePtr& production);
	void Invocation(InvocationPtr& production);
	void Unary(UnaryPtr& production);
	void AttributeParams(AttributeParamsPtr& production);
	void Attribute(AttributePtr& production);
	void Statement(StatementPtr& production);
	void Format(FormatPtr& production);
	void SubsetRange(SubsetRangePtr& production);
	void Subset(SubsetPtr& production);

	void Parse();

}; // end Parser

} // namespace


#endif

