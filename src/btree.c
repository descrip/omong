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
    int32_t num_keys;
    char *keys, *ids, *children;
};

struct BTree {
    struct BNode *root;
};

struct BNode *bnode_init(char *map) {
    struct BNode *bn = malloc(sizeof(struct BNode));
    bn->num_keys    = *(int32_t*) map;
    bn->keys        = map+4;
    bn->ids         = map+4 + sizeof(int32_t)*ORDER;
    bn->children    = map+4 + (sizeof(int32_t)*2)*ORDER;
    return bn;
}

void bnode_write_test(char *map) {
    int num_keys = 2;
    memcpy(map, &num_keys, sizeof(num_keys));
    int32_t keys[] = {23, 57};
    memcpy(map+4, keys, sizeof(keys));
    int32_t ids[] = {1, 2};
    memcpy(map+4 + sizeof(int32_t)*ORDER, ids, sizeof(ids));
}

int bnode_key_lower_bound(struct BNode *bn, int32_t key) {
    // TODO: linear search for now, replace with binary
    for (int i = 0; i < ORDER; ++i)
        if (key <= *(int32_t*) (bn->keys+i*sizeof(int32_t)))
            return i;
    return bn->num_keys;
}

static int32_t bnode_find(BNode *bn, int key) {
    int ind = bnode_key_lower_bound(bn, key);
    if (key == *(int32_t*) (bn->keys + ind*sizeof(int32_t)))
        return *(int32_t*) (bn->ids + ind*sizeof(int32_t));
    else {
        int child = *(int32_t*) (bn->children + ind*sizeof(int32_t));
        if (child == 0) return -1;
        else return bnode_find(child);
    }
}

int btree_find(BTree *bt, int key) {

}

int main() {
    int fd = open("test.bin", O_RDWR);
    int page_size = sysconf(_SC_PAGE_SIZE);

    char *map = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);

    // bnode_write_test(map);
    struct BNode *bn = bnode_init(map);

    int ind = bnode_key_lower_bound(bn, 22);
    printf("%d\n", ind);

    assert(munmap(map, sizeof(int32_t)*2) == 0);
    close(fd);

    return 0;
}
