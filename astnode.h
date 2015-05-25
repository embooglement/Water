#ifndef _ASTNODE_H_
#define _ASTNODE_H_

#include <iostream>

// TODO: eventually this will have a virtual codegen function
class ASTNode {
public:
	virtual void output(std::ostream& out) const = 0;
	virtual ~ASTNode() {}
};

#endif
