#define _XOPEN_SOURCE 500

#include "fdmap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <memory>
#include <unistd.h>

/* for raii, exposes file descriptor int completely */
class FileDescriptor {
  int fd;

public:
  FileDescriptor(const std::string &filename);
  ~FileDescriptor();

  int getFile() const;
  std::unique_ptr<FileDescriptorMap> makeMap(off_t offset) const;
  size_t getPageSize() const;
};
