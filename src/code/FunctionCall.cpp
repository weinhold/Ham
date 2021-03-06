/*
 * Copyright 2010-2013, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "code/FunctionCall.h"

#include "code/DumpContext.h"
#include "code/EvaluationContext.h"
#include "code/EvaluationException.h"
#include "code/Rule.h"
#include "code/RuleInstructions.h"
#include "data/TargetPool.h"
#include "util/Constants.h"


namespace ham {
namespace code {


FunctionCall::FunctionCall(Node* function)
	:
	fFunction(function),
	fArguments()
{
	fFunction->AcquireReference();
}


FunctionCall::FunctionCall(Node* function, const NodeList& arguments)
	:
	fFunction(function),
	fArguments(arguments)
{
	fFunction->AcquireReference();

	for (ArgumentList::iterator it = fArguments.begin(); it != fArguments.end();
		++it) {
		(*it)->AcquireReference();
	}
}


FunctionCall::~FunctionCall()
{
	fFunction->ReleaseReference();

	for (ArgumentList::iterator it = fArguments.begin(); it != fArguments.end();
		++it) {
		(*it)->ReleaseReference();
	}
}


StringList
FunctionCall::Evaluate(EvaluationContext& context)
{
	// check call depth
	size_t callDepth = context.RuleCallDepth();
	if (callDepth >= util::kRuleCallDepthLimit) {
		std::stringstream message;
		message << "Reached rule call depth limit ("
			<< util::kRuleCallDepthLimit << ")";
		throw EvaluationException(message.str());
	}
	context.SetRuleCallDepth(callDepth + 1);

	// evaluate arguments
	StringListList arguments;
	size_t argumentCount = fArguments.size();
	arguments.resize(argumentCount);

	size_t argumentIndex = 0;
	for (ArgumentList::iterator it = fArguments.begin(); it != fArguments.end();
		++it) {
		arguments[argumentIndex++] = (*it)->Evaluate(context);
	}

	// Call functions with arguments and concatenate the results.
	StringList result;
	StringList functions = fFunction->Evaluate(context);
	size_t functionCount = functions.Size();
	RulePool& rulePool = context.Rules();
	data::TargetList targets;
		// lazily initialized when needed
	data::TargetList sourceTargets;
		// lazily initialized when needed

	for (size_t i = 0; i < functionCount; i++) {
		Rule* function = rulePool.Lookup(functions.ElementAt(i));
		if (function == NULL) {
			context.ErrorOutput() << "warning: unknown rule "
				<< functions.ElementAt(i) << std::endl;
			continue;
		}

		// If the rule has actions, add respective action calls to the targets.
		if (data::RuleActions* actions = function->Actions()) {
			// lazily resolve the targets
			if (targets.empty() && argumentCount > 0 && !arguments[0].IsEmpty())
				context.Targets().LookupOrCreate(arguments[0], targets);

			// lazily resolve the source targets
			if (sourceTargets.empty() && argumentCount > 1
				&& !arguments[1].IsEmpty()) {
				context.Targets().LookupOrCreate(arguments[1], sourceTargets);
			}

			util::Reference<data::RuleActionsCall> actionsCall(
				new data::RuleActionsCall(actions, targets, sourceTargets),
				true);
			for (data::TargetList::iterator it = targets.begin();
				it != targets.end(); ++it) {
				(*it)->AddActionsCall(actionsCall.Get());
			}
		}

		// execute rule instructions (if any)
		if (RuleInstructions* instructions = function->Instructions())
			result.Append(instructions->Evaluate(context, arguments));
	}

	// reset call depth
	context.SetRuleCallDepth(callDepth);

	return result;
}


code::Node*
FunctionCall::Visit(NodeVisitor& visitor)
{
	if (visitor.VisitNode(this))
		return this;

	if (Node* result = fFunction->Visit(visitor))
		return result;

	for (ArgumentList::const_iterator it = fArguments.begin();
			it != fArguments.end(); ++it) {
		if (Node* result = (*it)->Visit(visitor))
			return result;
	}

	return NULL;
}


void
FunctionCall::Dump(DumpContext& context) const
{
	context << "FunctionCall(\n";
	context.BeginChildren();

	fFunction->Dump(context);

	for (ArgumentList::const_iterator it = fArguments.begin();
			it != fArguments.end(); ++it) {
		(*it)->Dump(context);
	}

	context.EndChildren();
	context << ")\n";
}


}	// namespace code
}	// namespace ham
