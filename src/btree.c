#define _XOPEN_SOURCE 500

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const int ORDER = 340;

struct BNode {
  void *map;
  uint32_t *num_keys;
  int32_t *keys;
  uint32_t *ids;
  uint32_t *children;
};

struct BTree {
  struct BNode *root;
};

struct BNode *bnode_init(int fd, uint32_t offset) {
  struct BNode *bn = malloc(sizeof(struct BNode));

  size_t page_size = sysconf(_SC_PAGE_SIZE);
  bn->map = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
      MAP_SHARED, fd, offset);

  bn->num_keys  = (uint32_t*) bn->map;
  bn->keys      = (int32_t*) (bn->map+4);
  bn->ids       = (uint32_t*) (bn->map+4 + sizeof(int32_t)*ORDER);
  bn->children  = (uint32_t*)
    (bn->map+4 + sizeof(int32_t)*ORDER + sizeof(uint32_t)*ORDER);

  return bn;
}

void bnode_destroy(struct BNode *bn) {
  assert(munmap(bn->map, sizeof(int32_t)*2) == 0);
}

void bnode_write_test(void *map) {
  int num_keys = 2;
  memcpy(map, &num_keys, sizeof(num_keys));
  int32_t keys[] = {23, 57};
  memcpy(map+4, keys, sizeof(keys));
  uint32_t ids[] = {1, 2};
  memcpy(map+4 + sizeof(int32_t)*ORDER, ids, sizeof(ids));
}

int bnode_key_lower_bound(struct BNode *bn, int32_t key) {
  // TODO: linear search for now, replace with binary
  for (size_t i = 0; i < *bn->num_keys; ++i)
    if (key <= bn->keys[i])
      return i;
  return *bn->num_keys;
}

/*
static int32_t bnode_find(struct BNode *bn, int key) {
  int ind = bnode_key_lower_bound(bn, key);
  if (key == *(int32_t*) (bn->keys + ind*sizeof(int32_t)))
    return *(int32_t*) (bn->ids + ind*sizeof(int32_t));
  else {
    int child = *(int32_t*) (bn->children + ind*sizeof(int32_t));
    if (child == 0) return -1;
    else return bnode_find(child);
  }
}
*/

/*
int btree_find(BTree *bt, int key) {

}
*/

int main() {
  int fd = open("test.bin", O_RDWR);

  // bnode_write_test(map);
  struct BNode *bn = bnode_init(fd, 0);

  int ind = bnode_key_lower_bound(bn, 99);
  printf("%d\n", ind);

  bnode_destroy(bn);
  close(fd);

  return 0;
}
