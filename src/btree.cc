#include "btree.h"

BTree::BTree(const std::string &filename)
  : fd{new FileDescriptor{filename}},
    root{std::move(fd->makeMap(0))} {}
