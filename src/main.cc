#include "bnode.h"

#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream> // TODO testing

void bNodeTest(int fd) {
  std::unique_ptr<BNode> bn = std::make_unique<BNode>(fd, 0);

  for (int i = 0; i < bn->getNumKeys(); ++i)
    std::cout << bn->keys[i] << '\n';
  /* 23 57 */

  std::cout << bn->lowerBound(12) << '\n';    // 0
}

int main() {
  int fd = open("test.bin", O_RDWR);
  bNodeTest(fd);
  close(fd);
}
