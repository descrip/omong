#include <stdio.h>
#include <math.h>

#define BLOCK_SIZE 512      // in bytes
#define MIN_ORDER 128

enum Type {
    DOUBLE      = '\x01',
    INT         = '\x10'
};

enum NodeType { ROOT, INTERNAL, EXTERNAL };

struct OrdType {
    enum Type type;
    int ord;
};

struct BTree {
    struct BNode * root;
    size_t block_size;
    size_t key_size;
    size_t header_size;
    int order;
    int blocks_per_node;
    struct OrdType index[];
};

struct BNode {
    int addr;

    enum NodeType type;
    int num_keys;
};

size_t type_size(enum Type t) {
    switch (t) {
    case INT:
        return sizeof(int);
    case DOUBLE:
        return sizeof(double);
    }
}

int type_cmp(void * a, void * b, enum Type t) {
    switch (t) {
    case INT:
        return *(int*)a < *(int*)b;
    case DOUBLE:
        return *(double*)a < *(double*)b;
    }
}

void get_dimensions(size_t block_size, size_t key_size, size_t header_size,
                    int * order, int * blocks_per_node) {
    *blocks_per_node =
        ceil(((double)(key_size + sizeof(size_t)) * MIN_ORDER + header_size +
              sizeof(size_t)) / block_size);
    *order =
        floor(((double)*blocks_per_node * block_size - header_size -
               sizeof(size_t)) / (key_size + sizeof(size_t)));
}

enum NodeType bnode_get_type(struct BTree * bt, struct BNode * bn) { }

int bnode_is_full(struct BTree * bt, struct BNode * bn) { }

// leaves file pointing to start of data
void bnode_fread(struct BTree * bt, struct BNode * bn, FILE * file) {
}

// test basic file io + casting
void test1() {
    int x = 9001;
    FILE * file;
    file = fopen("test.bin", "r+b");
    fwrite(&x, sizeof(x), 1, file);
    rewind(file);
    int y;
    fread(&y, sizeof(y), 1, file);
    printf("%d\n", y);
    int z = 230909;
    printf("%d\n", type_cmp(&y, &z, INT));
    fclose(file);
}

// test get_dimensions
void test2() {
    printf("%lu %lu %lu\n", sizeof(size_t), sizeof(int), sizeof(enum NodeType));
    int o, b;
    get_dimensions(BLOCK_SIZE, sizeof(double),
                   sizeof(int) + sizeof(enum NodeType), &o, &b);
    printf("%d %d\n", o, b);
}

// test fseek
void test3() {
    FILE * file = fopen("test.bin", "r+b");
    fseek(file, 20, SEEK_SET);
    int x = 9091;
    fwrite(&x, sizeof(x), 1, file);
    fclose(file);
}

// writes test node to file
void test4() {
    FILE * file = fopen("test.bin", "r+b");
    printf("%lu %d\n", sizeof(enum NodeType), ROOT);
    enum NodeType tmp_nt = ROOT;
    fwrite(&tmp_nt, sizeof(enum NodeType), 1, file);
    int size = 5;
    fwrite(&size, sizeof(int), 1, file);
    double d[] = {9.92, 203.23001, 9999.2, 898988.112};
    fwrite(d, sizeof(double), size, file);
    fclose(file);
    // fwrite((char*)9.92, sizeof(double)
}

int main() {
    // test1();
    // test2();
    // test3();
    test4();
    return 0;
}
