#include "btree.h"

#include <iostream>

struct Params {
  using KeyType       = int32_t;
  using OffsetType    = uint64_t;
  using SizeType      = uint16_t;
};

int main() {
  FileDescriptor fd {"test.bin"};
  BTree<Params> bt {std::move(fd)};
#ifdef DEBUG
  bt.test();
#endif
}
