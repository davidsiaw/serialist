

#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"


namespace Serialist {


void Parser::SynErr(int n) {
	if (errDist >= minErrDist) errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const wchar_t* msg) {
	if (errDist >= minErrDist) errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::Get() {
	for (;;) {
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT) { ++errDist; break; }
		if (la->kind == _ddtSym) {
		}
		if (la->kind == _optionSym) {
		}

		if (dummyToken != t) {
			dummyToken->kind = t->kind;
			dummyToken->pos = t->pos;
			dummyToken->col = t->col;
			dummyToken->line = t->line;
			dummyToken->next = NULL;
			coco_string_delete(dummyToken->val);
			dummyToken->val = coco_string_create(t->val);
			t = dummyToken;
		}
		la = t;
	}
}

void Parser::Expect(int n) {
	if (la->kind==n) Get(); else { SynErr(n); }
}

void Parser::ExpectWeak(int n, int follow) {
	if (la->kind == n) Get();
	else {
		SynErr(n);
		while (!StartOf(follow)) Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol) {
	if (la->kind == n) {Get(); return true;}
	else if (StartOf(repFol)) {return false;}
	else {
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0))) {
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::Serialist() {
		serialist = std::make_shared<class Serialist>(); 
		unsigned curline = la->line, curcol = la->col; 
		ElementPtr element; 
		while (la->kind == 9 /* "[" */ || la->kind == 15 /* "format" */ || la->kind == 18 /* "subset" */) {
			Element(element);
			serialist->elements.push_back(element); 
		}
		serialist->_line = curline; serialist->_col = curcol; 
}

void Parser::Element(ElementPtr& production) {
		unsigned curline = la->line, curcol = la->col; 
		FormatPtr format; 
		SubsetPtr subset; 
		if (la->kind == 9 /* "[" */ || la->kind == 15 /* "format" */) {
			Format(format);
			production = format; 
		} else if (la->kind == 18 /* "subset" */) {
			Subset(subset);
			production = subset; 
		} else SynErr(21);
		production->_line = curline; production->_col = curcol; 
}

void Parser::TypeIdentifier(TypeIdentifierPtr& production) {
		production = std::make_shared<class TypeIdentifier>(); 
		unsigned curline = la->line, curcol = la->col; 
		Expect(_pascalcase);
		production->content = t->val; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::MemberIdentifier(MemberIdentifierPtr& production) {
		production = std::make_shared<class MemberIdentifier>(); 
		unsigned curline = la->line, curcol = la->col; 
		Expect(_camelcase);
		production->content = t->val; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::NumberLit(NumberLitPtr& production) {
		production = std::make_shared<class NumberLit>(); 
		unsigned curline = la->line, curcol = la->col; 
		Expect(_number);
		production->content = t->val; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::HexLit(HexLitPtr& production) {
		production = std::make_shared<class HexLit>(); 
		unsigned curline = la->line, curcol = la->col; 
		Expect(_hexinteger);
		production->content = t->val; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::CharLit(CharLitPtr& production) {
		production = std::make_shared<class CharLit>(); 
		unsigned curline = la->line, curcol = la->col; 
		Expect(_char);
		production->content = t->val; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::StringLit(StringLitPtr& production) {
		production = std::make_shared<class StringLit>(); 
		unsigned curline = la->line, curcol = la->col; 
		Expect(_string);
		production->content = t->val; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::NumberLiteral(NumberLiteralPtr& production) {
		production = std::make_shared<class NumberLiteral>(); 
		unsigned curline = la->line, curcol = la->col; 
		NumberLitPtr numberlit; 
		NumberLit(numberlit);
		production->numberlit = numberlit; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::HexLiteral(HexLiteralPtr& production) {
		production = std::make_shared<class HexLiteral>(); 
		unsigned curline = la->line, curcol = la->col; 
		HexLitPtr hexlit; 
		HexLit(hexlit);
		production->hexlit = hexlit; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::MemberName(MemberNamePtr& production) {
		production = std::make_shared<class MemberName>(); 
		unsigned curline = la->line, curcol = la->col; 
		MemberIdentifierPtr memberidentifier; 
		MemberIdentifier(memberidentifier);
		production->memberidentifier = memberidentifier; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::CharLiteral(CharLiteralPtr& production) {
		production = std::make_shared<class CharLiteral>(); 
		unsigned curline = la->line, curcol = la->col; 
		CharLitPtr charlit; 
		CharLit(charlit);
		production->charlit = charlit; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::StringLiteral(StringLiteralPtr& production) {
		production = std::make_shared<class StringLiteral>(); 
		unsigned curline = la->line, curcol = la->col; 
		StringLitPtr stringlit; 
		StringLit(stringlit);
		production->stringlit = stringlit; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::ArrayLiteral(ArrayLiteralPtr& production) {
		production = std::make_shared<class ArrayLiteral>(); 
		unsigned curline = la->line, curcol = la->col; 
		ExpressionPtr expression; 
		Expect(9 /* "[" */);
		Expression(expression);
		production->expressions.push_back(expression); 
		while (la->kind == 10 /* "," */) {
			Get();
			Expression(expression);
			production->expressions.push_back(expression); 
		}
		Expect(11 /* "]" */);
		production->_line = curline; production->_col = curcol; 
}

void Parser::Expression(ExpressionPtr& production) {
		production = std::make_shared<class Expression>(); 
		unsigned curline = la->line, curcol = la->col; 
		UnaryPtr unary; 
		Unary(unary);
		production->unary = unary; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::BracketedExpression(BracketedExpressionPtr& production) {
		production = std::make_shared<class BracketedExpression>(); 
		unsigned curline = la->line, curcol = la->col; 
		ExpressionPtr expression; 
		Expect(12 /* "(" */);
		Expression(expression);
		production->expression = expression; 
		Expect(13 /* ")" */);
		production->_line = curline; production->_col = curcol; 
}

void Parser::SimpleLiteral(SimpleLiteralPtr& production) {
		unsigned curline = la->line, curcol = la->col; 
		NumberLiteralPtr numberliteral; 
		HexLiteralPtr hexliteral; 
		CharLiteralPtr charliteral; 
		if (la->kind == _number) {
			NumberLiteral(numberliteral);
			production = numberliteral; 
		} else if (la->kind == _hexinteger) {
			HexLiteral(hexliteral);
			production = hexliteral; 
		} else if (la->kind == _char) {
			CharLiteral(charliteral);
			production = charliteral; 
		} else SynErr(22);
		production->_line = curline; production->_col = curcol; 
}

void Parser::Primary(PrimaryPtr& production) {
		unsigned curline = la->line, curcol = la->col; 
		SimpleLiteralPtr simpleliteral; 
		StringLiteralPtr stringliteral; 
		ArrayLiteralPtr arrayliteral; 
		MemberNamePtr membername; 
		BracketedExpressionPtr bracketedexpression; 
		if (la->kind == _number || la->kind == _hexinteger || la->kind == _char) {
			SimpleLiteral(simpleliteral);
			production = simpleliteral; 
		} else if (la->kind == _string) {
			StringLiteral(stringliteral);
			production = stringliteral; 
		} else if (la->kind == 9 /* "[" */) {
			ArrayLiteral(arrayliteral);
			production = arrayliteral; 
		} else if (la->kind == _camelcase) {
			MemberName(membername);
			production = membername; 
		} else if (la->kind == 12 /* "(" */) {
			BracketedExpression(bracketedexpression);
			production = bracketedexpression; 
		} else SynErr(23);
		production->_line = curline; production->_col = curcol; 
}

void Parser::FunctionInvoke(FunctionInvokePtr& production) {
		production = std::make_shared<class FunctionInvoke>(); 
		unsigned curline = la->line, curcol = la->col; 
		ExpressionPtr expression; 
		Expect(12 /* "(" */);
		Expression(expression);
		production->expressions.push_back(expression); 
		while (la->kind == 10 /* "," */) {
			Get();
			Expression(expression);
			production->expressions.push_back(expression); 
		}
		Expect(13 /* ")" */);
		production->_line = curline; production->_col = curcol; 
}

void Parser::Invocation(InvocationPtr& production) {
		unsigned curline = la->line, curcol = la->col; 
		FunctionInvokePtr functioninvoke; 
		FunctionInvoke(functioninvoke);
		production = functioninvoke; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::Unary(UnaryPtr& production) {
		production = std::make_shared<class Unary>(); 
		unsigned curline = la->line, curcol = la->col; 
		PrimaryPtr primary; 
		InvocationPtr invocation; 
		Primary(primary);
		production->primary = primary; 
		while (la->kind == 12 /* "(" */) {
			Invocation(invocation);
			production->invocations.push_back(invocation); 
		}
		production->_line = curline; production->_col = curcol; 
}

void Parser::AttributeParams(AttributeParamsPtr& production) {
		production = std::make_shared<class AttributeParams>(); 
		unsigned curline = la->line, curcol = la->col; 
		ExpressionPtr expression; 
		Expect(14 /* ":" */);
		Expression(expression);
		production->expressions.push_back(expression); 
		while (la->kind == 10 /* "," */) {
			Get();
			Expression(expression);
			production->expressions.push_back(expression); 
		}
		production->_line = curline; production->_col = curcol; 
}

void Parser::Attribute(AttributePtr& production) {
		production = std::make_shared<class Attribute>(); 
		unsigned curline = la->line, curcol = la->col; 
		TypeIdentifierPtr typeidentifier; 
		AttributeParamsPtr attributeparams; 
		Expect(9 /* "[" */);
		TypeIdentifier(typeidentifier);
		production->typeidentifier = typeidentifier; 
		if (la->kind == 14 /* ":" */) {
			AttributeParams(attributeparams);
			production->attributeparams.push_back(attributeparams); 
		}
		Expect(11 /* "]" */);
		production->_line = curline; production->_col = curcol; 
}

void Parser::Statement(StatementPtr& production) {
		production = std::make_shared<class Statement>(); 
		unsigned curline = la->line, curcol = la->col; 
		AttributePtr attribute; 
		TypeIdentifierPtr typeidentifier; 
		MemberIdentifierPtr memberidentifier; 
		while (la->kind == 9 /* "[" */) {
			Attribute(attribute);
			production->attributes.push_back(attribute); 
		}
		TypeIdentifier(typeidentifier);
		production->typeidentifier = typeidentifier; 
		MemberIdentifier(memberidentifier);
		production->memberidentifier = memberidentifier; 
		production->_line = curline; production->_col = curcol; 
}

void Parser::Format(FormatPtr& production) {
		production = std::make_shared<class Format>(); 
		unsigned curline = la->line, curcol = la->col; 
		AttributePtr attribute; 
		TypeIdentifierPtr typeidentifier; 
		StatementPtr statement; 
		while (la->kind == 9 /* "[" */) {
			Attribute(attribute);
			production->attributes.push_back(attribute); 
		}
		Expect(15 /* "format" */);
		TypeIdentifier(typeidentifier);
		production->typeidentifier = typeidentifier; 
		Expect(16 /* "{" */);
		while (la->kind == _pascalcase || la->kind == 9 /* "[" */) {
			Statement(statement);
			production->statements.push_back(statement); 
		}
		Expect(17 /* "}" */);
		production->_line = curline; production->_col = curcol; 
}

void Parser::SubsetRange(SubsetRangePtr& production) {
		production = std::make_shared<class SubsetRange>(); 
		unsigned curline = la->line, curcol = la->col; 
		SimpleLiteralPtr simpleliteral; 
		SimpleLiteral(simpleliteral);
		production->simpleliterals.push_back(simpleliteral); 
		while (la->kind == 10 /* "," */) {
			Get();
			SimpleLiteral(simpleliteral);
			production->simpleliterals.push_back(simpleliteral); 
		}
		production->_line = curline; production->_col = curcol; 
}

void Parser::Subset(SubsetPtr& production) {
		production = std::make_shared<class Subset>(); 
		unsigned curline = la->line, curcol = la->col; 
		TypeIdentifierPtr typeidentifier; 
		SubsetRangePtr subsetrange; 
		Expect(18 /* "subset" */);
		TypeIdentifier(typeidentifier);
		production->typeidentifiers.push_back(typeidentifier); 
		Expect(14 /* ":" */);
		TypeIdentifier(typeidentifier);
		production->typeidentifiers.push_back(typeidentifier); 
		Expect(19 /* "=" */);
		SubsetRange(subsetrange);
		production->subsetrange = subsetrange; 
		production->_line = curline; production->_col = curcol; 
}




// If the user declared a method Init and a mehtod Destroy they should
// be called in the contructur and the destructor respctively.
//
// The following templates are used to recognize if the user declared
// the methods Init and Destroy.

template<typename T>
struct ParserInitExistsRecognizer {
	template<typename U, void (U::*)() = &U::Init>
	struct ExistsIfInitIsDefinedMarker{};

	struct InitIsMissingType {
		char dummy1;
	};
	
	struct InitExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static InitIsMissingType is_here(...);

	// exist only if ExistsIfInitIsDefinedMarker is defined
	template<typename U>
	static InitExistsType is_here(ExistsIfInitIsDefinedMarker<U>*);

	enum { InitExists = (sizeof(is_here<T>(NULL)) == sizeof(InitExistsType)) };
};

template<typename T>
struct ParserDestroyExistsRecognizer {
	template<typename U, void (U::*)() = &U::Destroy>
	struct ExistsIfDestroyIsDefinedMarker{};

	struct DestroyIsMissingType {
		char dummy1;
	};
	
	struct DestroyExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static DestroyIsMissingType is_here(...);

	// exist only if ExistsIfDestroyIsDefinedMarker is defined
	template<typename U>
	static DestroyExistsType is_here(ExistsIfDestroyIsDefinedMarker<U>*);

	enum { DestroyExists = (sizeof(is_here<T>(NULL)) == sizeof(DestroyExistsType)) };
};

// The folloing templates are used to call the Init and Destroy methods if they exist.

// Generic case of the ParserInitCaller, gets used if the Init method is missing
template<typename T, bool = ParserInitExistsRecognizer<T>::InitExists>
struct ParserInitCaller {
	static void CallInit(T *t) {
		// nothing to do
	}
};

// True case of the ParserInitCaller, gets used if the Init method exists
template<typename T>
struct ParserInitCaller<T, true> {
	static void CallInit(T *t) {
		t->Init();
	}
};

// Generic case of the ParserDestroyCaller, gets used if the Destroy method is missing
template<typename T, bool = ParserDestroyExistsRecognizer<T>::DestroyExists>
struct ParserDestroyCaller {
	static void CallDestroy(T *t) {
		// nothing to do
	}
};

// True case of the ParserDestroyCaller, gets used if the Destroy method exists
template<typename T>
struct ParserDestroyCaller<T, true> {
	static void CallDestroy(T *t) {
		t->Destroy();
	}
};

void Parser::Parse() {
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	Get();
	Serialist();
	Expect(0);
}

Parser::Parser(Scanner *scanner) {
	maxT = 20;

	ParserInitCaller<Parser>::CallInit(this);
	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	errors = new Errors();
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[1][22] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x}
	};



	return set[s][la->kind];
}

Parser::~Parser() {
	ParserDestroyCaller<Parser>::CallDestroy(this);
	delete errors;
	delete dummyToken;
}

Errors::Errors() {
	count = 0;
}

void Errors::SynErr(int line, int col, int n) {
	wchar_t* s;
	switch (n) {
			case 0: s = coco_string_create(L"EOF expected"); break;
			case 1: s = coco_string_create(L"pascalcase expected"); break;
			case 2: s = coco_string_create(L"camelcase expected"); break;
			case 3: s = coco_string_create(L"number expected"); break;
			case 4: s = coco_string_create(L"hexinteger expected"); break;
			case 5: s = coco_string_create(L"string expected"); break;
			case 6: s = coco_string_create(L"badString expected"); break;
			case 7: s = coco_string_create(L"char expected"); break;
			case 8: s = coco_string_create(L"endOfLine expected"); break;
			case 9: s = coco_string_create(L"\"[\" expected"); break;
			case 10: s = coco_string_create(L"\",\" expected"); break;
			case 11: s = coco_string_create(L"\"]\" expected"); break;
			case 12: s = coco_string_create(L"\"(\" expected"); break;
			case 13: s = coco_string_create(L"\")\" expected"); break;
			case 14: s = coco_string_create(L"\":\" expected"); break;
			case 15: s = coco_string_create(L"\"format\" expected"); break;
			case 16: s = coco_string_create(L"\"{\" expected"); break;
			case 17: s = coco_string_create(L"\"}\" expected"); break;
			case 18: s = coco_string_create(L"\"subset\" expected"); break;
			case 19: s = coco_string_create(L"\"=\" expected"); break;
			case 20: s = coco_string_create(L"??? expected"); break;
			case 21: s = coco_string_create(L"invalid Element"); break;
			case 22: s = coco_string_create(L"invalid SimpleLiteral"); break;
			case 23: s = coco_string_create(L"invalid Primary"); break;

		default:
		{
			wchar_t format[20];
			coco_swprintf(format, 20, L"error %d", n);
			s = coco_string_create(format);
		}
		break;
	}
	throw ParserException(line, col, s);
	//wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const wchar_t *s) {
	throw ParserException(line, col, s);
	//wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	count++;
}

void Errors::Warning(int line, int col, const wchar_t *s) {
	warnings.push_back(ParserException(line, col, s));
	//wprintf(L"-- line %d col %d: %ls\n", line, col, s);
}

void Errors::Warning(const wchar_t *s) {
	warnings.push_back(ParserException(0, 0, s));
	//wprintf(L"%ls\n", s);
}

void Errors::Exception(const wchar_t* s) {
	throw ParserException(0, 0, s);
	//wprintf(L"%ls", s); 
	exit(1);
}

} // namespace

