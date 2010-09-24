/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "code/RuleDefinition.h"

#include "code/DumpContext.h"
#include "code/EvaluationContext.h"


using code::RuleDefinition;


RuleDefinition::RuleDefinition(const String& ruleName)
	:
	fRuleName(ruleName),
	fParameterNames(),
	fBlock(NULL)
{
}


StringList
RuleDefinition::Evaluate(EvaluationContext& context)
{
	// TODO: Define the rule!
	return kFalseStringList;
}


void
RuleDefinition::Dump(DumpContext& context) const
{
	context << "RuleDefinition(\"" << fRuleName << "\", (";

	for (StringList::const_iterator it = fParameterNames.begin();
		it != fParameterNames.end(); ++it) {
		if (it != fParameterNames.begin())
			context << ", ";
		context << *it;
	}

	context << ")\n";

	context.BeginChildren();

	fBlock->Dump(context);

	context.EndChildren();
	context << ")\n";
}