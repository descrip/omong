#include "btree.h"

#include <iostream>

struct Params {
  using KeyType       = int32_t;
  using OffsetType    = uint64_t;
  using SizeType      = uint16_t;

  static const size_t
    /*
    InternalOrder     = 128,
    LeafOrder         = 256;
    */
    InternalOrder     = 4,
    LeafOrder         = 4;
};

int main() {
  FileDescriptor fd {"test.bin"};
  BTree<Params> bt {std::move(fd)};
  bt.test();
}
