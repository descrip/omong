#include "btree.h"

#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream> // TODO testing

int main() {
  BTree bt {"test.bin"};
}
