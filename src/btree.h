#ifndef BTREE_H
#define BTREE_H

#include "fd.h"
#include "bnode.h"

#include <memory>

class BTree {
public: // TODO testing
  std::unique_ptr<FileDescriptor> fd;
  BNode root;

public:
  BTree(const std::string &filename);
};

#endif
