#include "btree.h"

#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream> // TODO testing

int main() {
  BTree bt {"test.bin"};
  std::cout << bt.root.getNumKeys() << '\n';
  for (size_t i = 0; i < bt.root.getNumKeys(); ++i)
    std::cout << bt.root.keys[i] << '\n';
}
