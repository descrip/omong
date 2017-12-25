#define _XOPEN_SOURCE 500

#include "fdmap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <memory>
#include <unistd.h>
#include <cassert>

/* for raii, exposes file descriptor int completely */
class FileDescriptor {
  int fd;

public:
  FileDescriptor(const std::string &filename);
  ~FileDescriptor();

  /* don't allow copy, move */
  FileDescriptor(const FileDescriptor &other) = delete;
  FileDescriptor &operator=(const FileDescriptor &other) = delete;
  FileDescriptor(FileDescriptor &&other) = delete;
  FileDescriptor &operator=(FileDescriptor &&other) = delete;

  int getFile() const;
  std::unique_ptr<FileDescriptorMap> makeMap(off_t offset) const;
  size_t getPageSize() const;
};
