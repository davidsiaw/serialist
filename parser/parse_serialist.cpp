
#include "parse_serialist.hpp"

/*
	WARNING: This file is generated using ruco. Please modify the .ruco file if you wish to change anything
	https://github.com/davidsiaw/ruco
*/

namespace Serialist
{
	SerialistPtr Parse(std::string sourceFile)
	{
		std::shared_ptr<FILE> fp (fopen(sourceFile.c_str(), "r"), fclose);
		if (!fp)
		{
			throw FileNotFoundException();
		}
		std::shared_ptr<Scanner> scanner (new Scanner(fp.get()));
		std::shared_ptr<Parser> parser (new Parser(scanner.get()));
		parser->Parse();

		return parser->serialist;
	}

	picojson::object CompileSerialist(SerialistPtr pointer);
	picojson::object CompileTypeIdentifier(TypeIdentifierPtr pointer);
	picojson::object CompileMemberIdentifier(MemberIdentifierPtr pointer);
	picojson::object CompileNumberLit(NumberLitPtr pointer);
	picojson::object CompileCharLit(CharLitPtr pointer);
	picojson::object CompileStringLit(StringLitPtr pointer);
	picojson::object CompileIntegerLiteral(IntegerLiteralPtr pointer);
	picojson::object CompileMemberName(MemberNamePtr pointer);
	picojson::object CompileCharLiteral(CharLiteralPtr pointer);
	picojson::object CompileStringLiteral(StringLiteralPtr pointer);
	picojson::object CompileBracketedExpression(BracketedExpressionPtr pointer);
	picojson::object CompilePrimary(PrimaryPtr pointer);
	picojson::object CompileFunctionInvoke(FunctionInvokePtr pointer);
	picojson::object CompileInvocation(InvocationPtr pointer);
	picojson::object CompileUnary(UnaryPtr pointer);
	picojson::object CompileExpression(ExpressionPtr pointer);
	picojson::object CompileAttributeParams(AttributeParamsPtr pointer);
	picojson::object CompileAttribute(AttributePtr pointer);
	picojson::object CompileStatement(StatementPtr pointer);
	picojson::object CompileFormat(FormatPtr pointer);

	picojson::object CompileSerialist(SerialistPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"Serialist");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>3, :type=>:id}

		picojson::array formats;

		for(unsigned i=0; i<pointer->formats.size(); i++)
		{
			formats.push_back(picojson::value(CompileFormat(pointer->formats[i])));
		}

		object[L"formats"] = picojson::value(formats);




		return object;
	}

	picojson::object CompileTypeIdentifier(TypeIdentifierPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"TypeIdentifier");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:token}
		object[L"_token"] = picojson::value(pointer->content);




		return object;
	}

	picojson::object CompileMemberIdentifier(MemberIdentifierPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"MemberIdentifier");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:token}
		object[L"_token"] = picojson::value(pointer->content);




		return object;
	}

	picojson::object CompileNumberLit(NumberLitPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"NumberLit");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:token}
		object[L"_token"] = picojson::value(pointer->content);




		return object;
	}

	picojson::object CompileCharLit(CharLitPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"CharLit");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:token}
		object[L"_token"] = picojson::value(pointer->content);




		return object;
	}

	picojson::object CompileStringLit(StringLitPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"StringLit");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:token}
		object[L"_token"] = picojson::value(pointer->content);




		return object;
	}

	picojson::object CompileIntegerLiteral(IntegerLiteralPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"IntegerLiteral");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->numberlit)
		{
			picojson::object numberlit;

			numberlit = CompileNumberLit(pointer->numberlit);

			object[L"numberlit"] = picojson::value(numberlit);
		}




		return object;
	}

	picojson::object CompileMemberName(MemberNamePtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"MemberName");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->memberidentifier)
		{
			picojson::object memberidentifier;

			memberidentifier = CompileMemberIdentifier(pointer->memberidentifier);

			object[L"memberidentifier"] = picojson::value(memberidentifier);
		}




		return object;
	}

	picojson::object CompileCharLiteral(CharLiteralPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"CharLiteral");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->charlit)
		{
			picojson::object charlit;

			charlit = CompileCharLit(pointer->charlit);

			object[L"charlit"] = picojson::value(charlit);
		}




		return object;
	}

	picojson::object CompileStringLiteral(StringLiteralPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"StringLiteral");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->stringlit)
		{
			picojson::object stringlit;

			stringlit = CompileStringLit(pointer->stringlit);

			object[L"stringlit"] = picojson::value(stringlit);
		}




		return object;
	}

	picojson::object CompileBracketedExpression(BracketedExpressionPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"BracketedExpression");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->expression)
		{
			picojson::object expression;

			expression = CompileExpression(pointer->expression);

			object[L"expression"] = picojson::value(expression);
		}




		return object;
	}

	picojson::object CompilePrimary(PrimaryPtr pointer)
	{
		picojson::object object;

		// variation
		object[L"_type"] = picojson::value(L"Primary");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		picojson::object content;
		switch(pointer->get_primary_type())
		{
			case INTEGERLITERAL_PRIMARY:
			{
				content = CompileIntegerLiteral(std::dynamic_pointer_cast<IntegerLiteral>(pointer));
				break;
			}
			case CHARLITERAL_PRIMARY:
			{
				content = CompileCharLiteral(std::dynamic_pointer_cast<CharLiteral>(pointer));
				break;
			}
			case STRINGLITERAL_PRIMARY:
			{
				content = CompileStringLiteral(std::dynamic_pointer_cast<StringLiteral>(pointer));
				break;
			}
			case MEMBERNAME_PRIMARY:
			{
				content = CompileMemberName(std::dynamic_pointer_cast<MemberName>(pointer));
				break;
			}
			case BRACKETEDEXPRESSION_PRIMARY:
			{
				content = CompileBracketedExpression(std::dynamic_pointer_cast<BracketedExpression>(pointer));
				break;
			}

		}

		object[L"_content"] = picojson::value(content);

		return object;
	}

	picojson::object CompileFunctionInvoke(FunctionInvokePtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"FunctionInvoke");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>3, :type=>:id}

		picojson::array expressions;

		for(unsigned i=0; i<pointer->expressions.size(); i++)
		{
			expressions.push_back(picojson::value(CompileExpression(pointer->expressions[i])));
		}

		object[L"expressions"] = picojson::value(expressions);




		return object;
	}

	picojson::object CompileInvocation(InvocationPtr pointer)
	{
		picojson::object object;

		// variation
		object[L"_type"] = picojson::value(L"Invocation");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		picojson::object content;
		switch(pointer->get_invocation_type())
		{
			case FUNCTIONINVOKE_INVOCATION:
			{
				content = CompileFunctionInvoke(std::dynamic_pointer_cast<FunctionInvoke>(pointer));
				break;
			}

		}

		object[L"_content"] = picojson::value(content);

		return object;
	}

	picojson::object CompileUnary(UnaryPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"Unary");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->primary)
		{
			picojson::object primary;

			primary = CompilePrimary(pointer->primary);

			object[L"primary"] = picojson::value(primary);
		}
		// {:count=>3, :type=>:id}

		picojson::array invocations;

		for(unsigned i=0; i<pointer->invocations.size(); i++)
		{
			invocations.push_back(picojson::value(CompileInvocation(pointer->invocations[i])));
		}

		object[L"invocations"] = picojson::value(invocations);




		return object;
	}

	picojson::object CompileExpression(ExpressionPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"Expression");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->unary)
		{
			picojson::object unary;

			unary = CompileUnary(pointer->unary);

			object[L"unary"] = picojson::value(unary);
		}




		return object;
	}

	picojson::object CompileAttributeParams(AttributeParamsPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"AttributeParams");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>3, :type=>:id}

		picojson::array expressions;

		for(unsigned i=0; i<pointer->expressions.size(); i++)
		{
			expressions.push_back(picojson::value(CompileExpression(pointer->expressions[i])));
		}

		object[L"expressions"] = picojson::value(expressions);




		return object;
	}

	picojson::object CompileAttribute(AttributePtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"Attribute");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>1, :type=>:id}
		if (pointer->typeidentifier)
		{
			picojson::object typeidentifier;

			typeidentifier = CompileTypeIdentifier(pointer->typeidentifier);

			object[L"typeidentifier"] = picojson::value(typeidentifier);
		}
		// {:count=>3, :type=>:id}

		picojson::array attributeparams;

		for(unsigned i=0; i<pointer->attributeparams.size(); i++)
		{
			attributeparams.push_back(picojson::value(CompileAttributeParams(pointer->attributeparams[i])));
		}

		object[L"attributeparams"] = picojson::value(attributeparams);




		return object;
	}

	picojson::object CompileStatement(StatementPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"Statement");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>3, :type=>:id}

		picojson::array attributes;

		for(unsigned i=0; i<pointer->attributes.size(); i++)
		{
			attributes.push_back(picojson::value(CompileAttribute(pointer->attributes[i])));
		}

		object[L"attributes"] = picojson::value(attributes);
		// {:count=>1, :type=>:id}
		if (pointer->typeidentifier)
		{
			picojson::object typeidentifier;

			typeidentifier = CompileTypeIdentifier(pointer->typeidentifier);

			object[L"typeidentifier"] = picojson::value(typeidentifier);
		}
		// {:count=>1, :type=>:id}
		if (pointer->memberidentifier)
		{
			picojson::object memberidentifier;

			memberidentifier = CompileMemberIdentifier(pointer->memberidentifier);

			object[L"memberidentifier"] = picojson::value(memberidentifier);
		}




		return object;
	}

	picojson::object CompileFormat(FormatPtr pointer)
	{
		picojson::object object;

		// normal
		object[L"_type"] = picojson::value(L"Format");
		object[L"_col"] = picojson::value((double)pointer->_col);
		object[L"_line"] = picojson::value((double)pointer->_line);
		

		// {:count=>3, :type=>:id}

		picojson::array attributes;

		for(unsigned i=0; i<pointer->attributes.size(); i++)
		{
			attributes.push_back(picojson::value(CompileAttribute(pointer->attributes[i])));
		}

		object[L"attributes"] = picojson::value(attributes);
		// {:count=>1, :type=>:id}
		if (pointer->typeidentifier)
		{
			picojson::object typeidentifier;

			typeidentifier = CompileTypeIdentifier(pointer->typeidentifier);

			object[L"typeidentifier"] = picojson::value(typeidentifier);
		}
		// {:count=>3, :type=>:id}

		picojson::array statements;

		for(unsigned i=0; i<pointer->statements.size(); i++)
		{
			statements.push_back(picojson::value(CompileStatement(pointer->statements[i])));
		}

		object[L"statements"] = picojson::value(statements);




		return object;
	}



	picojson::value Jsonify(SerialistPtr parseResult)
	{
		return picojson::value(CompileSerialist(parseResult));
	}

}

