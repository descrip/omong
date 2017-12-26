#include "bnode.h"

BNode::BNode(FileDescriptorMap &&tmp)
  : map{std::move(tmp)},
    numKeys{(ID_T*) map.getMap()},
    keys{(KEY_T*) (map.getMap() + HEADER_SIZE)},
    ids{(ID_T*) (map.getMap() + HEADER_SIZE + KEYS_SIZE)},
    children{(ID_T*) (map.getMap() + HEADER_SIZE + KEYS_SIZE + CHILDREN_SIZE)} {}

size_t BNode::lowerBound(KEY_T key) {
  // TODO: linear search for now, replace with binary
  for (size_t i = 0; i < getNumKeys(); ++i)
    if (key <= keys[i])
      return i;
  return getNumKeys();
}

