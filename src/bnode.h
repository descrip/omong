#define _XOPEN_SOURCE 500

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>

class BNode {
  /*
   * page               = 4096 bytes
   * KEY_T, ID_T  = 4 bytes
   * order              = 341
   *
   * +-----------------------------+
   * | 4     | ID_T | numKeys  |
   * +-----------------------------+
   * | 340*4 | KEY_T  | keys     |
   * +-----------------------------+
   * | 340*4 | ID_T | ids      |
   * +-----------------------------+
   * | 341*4 | ID_T | children |
   * +-----------------------------+
   *
   * = 4088 bytes, 8 free
   */
public:     // TODO testing
  using KEY_T = int32_t;
  using ID_T = uint32_t;

  static const size_t ORDER = 340;
  static const std::ptrdiff_t
    HEADER_SIZE     = sizeof(ID_T),
    KEYS_SIZE       = (ORDER-1) * sizeof(KEY_T),
    IDS_SIZE        = (ORDER-1) * sizeof(ID_T),
    CHILDREN_SIZE   = ORDER * sizeof(ID_T);

  char *map;
  ID_T *numKeys;
  KEY_T *keys;
  ID_T *ids;
  ID_T *children;

public:
  BNode(int fd, off_t offset);
  ~BNode();

  int getNumKeys() { return *numKeys; }

  int lowerBound(KEY_T key);
};
