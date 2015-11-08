#include <iostream>

#include "mark.h"

std::ostream& operator<<(std::ostream& out, Mark m) {
	switch (m) {
		case Mark::FLOW_STMT:
			out << "FLOW STATEMENT";
			break;
		case Mark::LOCK:
			out << "LOCK";
			break;
		case Mark::CALL:
			out << "CALL";
			break;
		case Mark::DISCARDABLE:
			out << "DISCARDABLE";
			break;
		case Mark::LSM_HOOK:
			out << "LSM HOOK";
			break;
		case Mark::LAST_AND_UNUSED_MARK:
			out << "<waiting for a mark>";
			break;
	}
	return out;
}

std::istream& operator>>(std::istream& in, Mark& m) {
    std::string buf;
    in >> buf;
    if (buf == "FLOW STATEMENT")
        m = Mark::FLOW_STMT;
    else if (buf == "LOCK")
        m = Mark::LOCK;
    else if (buf == "CALL")
        m = Mark::CALL;
    else if (buf == "DISCARDABLE")
        m = Mark::DISCARDABLE;
    else if (buf == "LSM_HOOK")
        m = Mark::LSM_HOOK;
    else
        m = Mark::LAST_AND_UNUSED_MARK;
	return in;
}
