#ifndef FDMAP_H
#define FDMAP_H

#include <sys/mman.h>
#include <cassert>

/* for raii, exposes the map completely */
class FileDescriptorMap {
  size_t size;
  char *map;

public:
  FileDescriptorMap(int fd, off_t offset, size_t size);
  FileDescriptorMap(size_t size, char *map);
  ~FileDescriptorMap();

  /* don't allow copy, move */
  FileDescriptorMap(const FileDescriptorMap &other) = delete;
  FileDescriptorMap &operator=(const FileDescriptorMap &other) = delete;
  FileDescriptorMap(FileDescriptorMap &&other) = delete;
  FileDescriptorMap &operator=(FileDescriptorMap &&other) = delete;

  char *getMap();
  size_t getSize() const;
};

#endif
