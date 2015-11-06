#ifndef MARK_H
#define MARK_H

#include <iostream>

enum class Mark {
  LSM_HOOK,
  LOCK,
  FLOW_STMT,
  CALL,
  DISCARDABLE,
  LAST_AND_UNUSED_MARK
};

std::ostream& operator<<(std::ostream& out, Mark m);

#endif
