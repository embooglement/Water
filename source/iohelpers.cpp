#include "iohelpers.h"

using namespace std;

namespace io {
	_Details::_Indent indent(unsigned int indent) {
		return {indent};
	}
}

ostream& operator<<(ostream& out, const io::_Details::_Indent& indenter) {
	for (unsigned int i = 0; i < indenter._indent; ++i) {
		out << "    ";
	}

	return out;
}
