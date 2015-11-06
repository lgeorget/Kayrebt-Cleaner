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

