#include "btree.h"

#include <iostream>

struct Params {
  using KeyType       = int32_t;
  using OffsetType    = uint64_t;
  using SizeType      = uint16_t;

  static const size_t
    InternalMinDeg    = 64,
    InternalOrder     = 2*InternalMinDeg,
    LeafMinDeg        = 128,
    LeafOrder         = 2*LeafMinDeg;
};

int main() {
  FileDescriptor fd {"test.bin"};
  BTree<Params> bt {std::move(fd)};
  bt.test();
}
