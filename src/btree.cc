#include "btree.h"

BTree::BTree(const std::string &filename)
  : fd{std::make_unique<FileDescriptor>(filename)},
    root{fd->makeMap(0)} {}
