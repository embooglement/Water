#ifndef _IO_H_
#define _IO_H_

#include <iostream>
#include <functional>

namespace io {
	namespace _Details {
		struct _Indent {
			unsigned int _indent;
		};
	}

	_Details::_Indent indent(unsigned int indent);
}

std::ostream& operator<<(std::ostream& out, const io::_Details::_Indent& indenter);

#endif
