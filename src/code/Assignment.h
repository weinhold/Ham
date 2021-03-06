/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef HAM_CODE_ASSIGMENT_H
#define HAM_CODE_ASSIGMENT_H


#include "code/Node.h"
#include "code/Defs.h"


namespace ham {
namespace code {


class Assignment : public Node {
public:
								Assignment(Node* left,
									AssignmentOperator operatorType,
									Node* right, Node* onTargets = NULL);
	virtual						~Assignment();

	virtual	StringList			Evaluate(EvaluationContext& context);
	virtual	Node*				Visit(NodeVisitor& visitor);
	virtual	void				Dump(DumpContext& context) const;

private:
			Node*				fLeft;
			Node*				fRight;
			Node*				fOnTargets;
			AssignmentOperator fOperator;
};


}	// namespace code
}	// namespace ham


#endif	// HAM_CODE_ASSIGMENT_H
