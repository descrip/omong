#define _XOPEN_SOURCE 500

#include "fdmap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <cassert>

// for raii, exposes file descriptor int completely
class FileDescriptor {
  int fd;
  bool hasFd;

public:
  FileDescriptor(const std::string &filename);
  ~FileDescriptor();

  FileDescriptor(const FileDescriptor &other) = delete;
  FileDescriptor &operator=(const FileDescriptor &other) = delete;

  FileDescriptor(FileDescriptor &&other) noexcept;
  FileDescriptor &operator=(FileDescriptor &&other);

  int getFile();
  FileDescriptorMap makeMap(off_t offset);
  size_t getPageSize();
};
