#include "btree.h"

#include <iostream>

struct Params {
  using KeyType       = int32_t;
  using OffsetType    = int64_t;
  using SizeType      = uint16_t;
};

int main() {
  FileDescriptor fd {"test.bin"};
  // fd must have size at least 8196
  BTree<Params> bt {std::move(fd)};
#ifdef DEBUG
  bt.test();
#endif
  std::cout << fd.size() << '\n';
}
