#include <cassert>

class BNode {
  void *map;
  uint32_t *num_keys;
  int32_t *keys;
  uint32_t *ids;
  uint32_t *children;

public:

  BNode(int fd, uint32_t offset);

};

BNode(int fd, uint32_t offset) :
  map(mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset)) {

}
