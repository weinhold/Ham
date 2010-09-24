/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "code/BinaryExpression.h"

#include "code/DumpContext.h"
#include "code/EvaluationContext.h"


namespace code {


template<typename Operator>
BinaryExpression<Operator>::BinaryExpression(Node* left, Node* right)
	:
	fLeft(left),
	fRight(right)
{
}


template<typename Operator>
StringList
BinaryExpression<Operator>::Evaluate(EvaluationContext& context)
{
	StringList leftList = fLeft->Evaluate(context);
	StringList rightList = fRight->Evaluate(context);

	return Operator::Do(fLeft->Evaluate(context),
		fRight->Evaluate(context));
}


template<typename Operator>
void
BinaryExpression<Operator>::Dump(DumpContext& context) const
{
	context << "BinaryExpressionNode[" << Operator::kSymbol << "](\n";
	context.BeginChildren();

	fLeft->Dump(context);
	fRight->Dump(context);

	context.EndChildren();
	context << ")\n";
}


// define and instantiate the specializations

#define HAM_DEFINE_OPERATOR_EXPRESSION(name, symbol, expression)		\
	struct name##Operator {												\
		static const char* const kSymbol;								\
																		\
		static StringList Do(const StringList& a, const StringList& b)	\
		{																\
			return expression ? kTrueStringList : kFalseStringList;		\
		}																\
	};																	\
																		\
	const char* const name##Operator::kSymbol = #symbol;				\
																		\
	template class BinaryExpression<name##Operator>;


HAM_DEFINE_OPERATOR_EXPRESSION(Equal,			=,	a == b)
HAM_DEFINE_OPERATOR_EXPRESSION(NotEqual,		!=,	a != b)
HAM_DEFINE_OPERATOR_EXPRESSION(Less,			<,	a < b)
HAM_DEFINE_OPERATOR_EXPRESSION(LessOrEqual,		<=,	a <= b)
HAM_DEFINE_OPERATOR_EXPRESSION(Greater,			>,	a > b)
HAM_DEFINE_OPERATOR_EXPRESSION(GreaterOrEqual,	>=,	a >= b)
HAM_DEFINE_OPERATOR_EXPRESSION(And,				&&,	!a.empty() && !b.empty())
HAM_DEFINE_OPERATOR_EXPRESSION(Or,				||,	!a.empty() || !b.empty())

#undef HAM_DEFINE_OPERATOR_EXPRESSION


}	// namespace code