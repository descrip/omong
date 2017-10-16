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

#define MIN_ORDER 128

// TODO: duplicates?

/*
 * off_t for offsets
 * size_t for sizes (which can be used as offsets)
 * use stdint.h for integer with sizes
 * wtf am i going to do for standardised double???
 */

typedef struct {
    int32_t num_keys;
} BNodeHeader;

typedef struct {
    BNodeHeader *header;
    char *keys;
    char *ids;
    char *children;
} BNode;

typedef struct {
    size_t header_size;
    size_t key_size;
    int order;
    BNode *root;
} BTree;

// TODO: ftruncate to pad file
size_t get_file_size(const char *filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

// TODO: static for now
int get_order() {
    return 256;
}

off_t bnode_get_keys_offset() {
    return 12;
}

off_t bnode_get_ids_offset() {
    return bnode_get_keys_offset() + sizeof(double) * get_order();
}

off_t bnode_get_children_offset() {
    return bnode_get_ids_offset() + sizeof(uint32_t) * get_order();
}

BNode *bnode_init(char *map) {
    BNode *bn = malloc(sizeof(BNode));
    bn->header = malloc(sizeof(BNodeHeader));
    bn->header->num_keys = *(int32_t*)map;
    bn->keys = map + bnode_get_keys_offset();
    bn->ids = map + bnode_get_ids_offset();
    bn->children = map + bnode_get_children_offset();
    return bn;
}

off_t btree_find_internal(BNode *bn, double key) {
    if (bn->header->num_keys == 0)
        return -1;
}

int bnode_key_lowerbound(BTree *bt, BNode *bn, double key) {
    size_t key_size = sizeof(double);       // TODO: hardcoded for now
    // TODO: linear search, replace with binary
    for (int i = 0; i < bn->header->num_keys; ++i)
        if (key <= *(double*)(bn->keys + key_size*i))
            return i;
    return bn->header->num_keys;
}

void write_test_bnode(char *map) {
    /*
    int x = 9002, y = 884;
    memcpy(map, &x, sizeof(int32_t));
    memcpy(map+sizeof(int32_t), &y, sizeof(int32_t));
    */

    int32_t num_keys = 2;
    memcpy(map, &num_keys, sizeof(int32_t));

    double keys[] = {9.2393, 999929.57};
    memcpy(map + bnode_get_keys_offset(), keys, sizeof(keys));
}

int main() {
    int fd = open("test.bin", O_RDWR);
    int page_size = sysconf(_SC_PAGE_SIZE);
    // printf("%lu\n", get_file_size("test.bin"));

    /*
    int x = 9001, y = 883;
    assert(pwrite(fd, &x, sizeof(int32_t), 0) != 0);
    assert(pwrite(fd, &y, sizeof(int32_t), sizeof(int32_t)) != 0);
    */

    char *map = mmap(NULL, sizeof(int32_t)*2, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);

    write_test_bnode(map);
    BNode *bn = bnode_init(map);
    printf("%d\n", bn->header->num_keys);
    printf("%lf\n", *(double*)bn->keys);
    printf("%lf\n", *(double*)(bn->keys+sizeof(double)));

    printf("%d\n", bnode_key_lowerbound(NULL, bn, 999929.58));

    BTree *bt = malloc(sizeof(BTree));
    bt->root = bn;

    /*
    int32_t a = *(int32_t*) map,
            b = *(int32_t*) (map+sizeof(int32_t));
    printf("%d %d\n", a, b);
    */

    assert(munmap(map, sizeof(int32_t)*2) == 0);
    close(fd);

    return 0;
}
