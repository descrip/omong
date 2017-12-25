#include "bnode.h"

BNode::BNode(std::unique_ptr<FileDescriptorMap> map)
  : map{std::move(map)},
    numKeys{(ID_T*) map.get()},
    keys{(KEY_T*) (map.get() + HEADER_SIZE)},
    ids{(ID_T*) (map.get() + HEADER_SIZE + KEYS_SIZE)},
    children{(ID_T*) (map.get() + HEADER_SIZE + KEYS_SIZE + CHILDREN_SIZE)} {}

int BNode::lowerBound(KEY_T key) {
  // TODO: linear search for now, replace with binary
  for (size_t i = 0; i < getNumKeys(); ++i)
    if (key <= keys[i])
      return i;
  return getNumKeys();
}

