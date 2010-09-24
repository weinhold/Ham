/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef HAM_CODE_ASSIGMENT_H
#define HAM_CODE_ASSIGMENT_H


#include "code/List.h"


namespace code {


enum AssignmentOperator {
	ASSIGNMENT_OPERATOR_ASSIGN,
	ASSIGNMENT_OPERATOR_APPEND,
	ASSIGNMENT_OPERATOR_DEFAULT
};


class Assignment : public Node {
public:
								Assignment(Node* left,
									AssignmentOperator operatorType,
									List* right, List* onTargets = NULL);

	virtual	StringList			Evaluate(EvaluationContext& context);

	virtual	void				Dump(DumpContext& context) const;

private:
			Node*				fLeft;
			List*				fRight;
			List*				fOnTargets;
			AssignmentOperator fOperator;
};


}	// namespace code


#endif	// HAM_CODE_ASSIGMENT_H