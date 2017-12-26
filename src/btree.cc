#include "btree.h"

BTree::BTree(const std::string &filename)
  : fd{filename},
    root{fd.makeMap(0)} {}
