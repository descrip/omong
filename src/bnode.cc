#include "bnode.h"

BNode::BNode(int fd, off_t offset)
  : map{static_cast<char *>(mmap(NULL, sysconf(_SC_PAGE_SIZE),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset))},
    numKeys{(ID_T*) map},
    keys{(KEY_T*) (map + HEADER_SIZE)},
    ids{(ID_T*) (map + HEADER_SIZE + KEYS_SIZE)},
    children{(ID_T*) (map + HEADER_SIZE + KEYS_SIZE + CHILDREN_SIZE)} {}

BNode::~BNode() {
  assert(munmap(map, sizeof(KEY_T)*2) == 0);
}

int BNode::lowerBound(KEY_T key) {
  // TODO: linear search for now, replace with binary
  for (size_t i = 0; i < getNumKeys(); ++i)
    if (key <= keys[i])
      return i;
  return getNumKeys();
}

