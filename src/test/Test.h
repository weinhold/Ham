/*
 * Copyright 2012, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef HAM_TEST_TEST_H
#define HAM_TEST_TEST_H


#include <string>


namespace ham {
namespace test {


class TestSuite;


class Test {
public:
								Test(const std::string& name);
	virtual						~Test();

			const std::string&	Name() const
									{ return fName; }
			std::string			FullyQualifiedName() const;

			TestSuite*			GetTestSuite() const
									{ return fTestSuite; }
			void				SetTestSuite(TestSuite* testSuite)
									{ fTestSuite = testSuite; }

private:
			std::string			fName;
			TestSuite*			fTestSuite;
};


} // namespace test
} // namespace ham


#endif // HAM_TEST_TEST_H
