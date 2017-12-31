#define _XOPEN_SOURCE 500

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <cassert>
#include <sys/mman.h>
#include <cassert>

class FileDescriptorMap {
  size_t size_;
  char *map_;
  bool hasMap_;

public:
  FileDescriptorMap(int fd, off_t offset, size_t size);
  FileDescriptorMap(size_t size, char *map);
  ~FileDescriptorMap();

  FileDescriptorMap(const FileDescriptorMap &other) = delete;
  FileDescriptorMap &operator=(const FileDescriptorMap &other) = delete;

  FileDescriptorMap(FileDescriptorMap &&other) noexcept;
  FileDescriptorMap &operator=(FileDescriptorMap &&other);

  char *get();
  size_t size() const;
};

class FileDescriptor {
  int fd_;
  bool hasFd_;
  struct stat stats_;

public:
  FileDescriptor(const std::string &filename);
  ~FileDescriptor();

  FileDescriptor(const FileDescriptor &other) = delete;
  FileDescriptor &operator=(const FileDescriptor &other) = delete;

  FileDescriptor(FileDescriptor &&other) noexcept;
  FileDescriptor &operator=(FileDescriptor &&other);

  int fd();
  FileDescriptorMap loadMap(off_t offset);
  size_t getPageSize();
  void truncate(off_t size);
  off_t size();

private:
  void getStats();
};
