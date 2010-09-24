/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "code/Leaf.h"

#include "code/DumpContext.h"
#include "code/EvaluationContext.h"


using code::Leaf;


Leaf::Leaf(const String& string)
	:
	fString(string)
{
}

StringList
Leaf::Evaluate(EvaluationContext& context)
{
	// TODO: Evalutate the string (variable substitution...).
	StringList list;
	list.push_back(fString);
	return list;
}

void
Leaf::Dump(DumpContext& context) const
{
	context << "CodeLeaf(\"" << fString << "\")\n";
}