#include "fdmap.h"

FileDescriptorMap::FileDescriptorMap(int fd, off_t offset, size_t size)
  : size{size},
    map{static_cast<char *>(mmap(nullptr, size, PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, offset))},
    hasMap{true} {}

FileDescriptorMap::FileDescriptorMap(size_t size, char *map)
  : size{size}, map{map}, hasMap{true} {}

FileDescriptorMap::~FileDescriptorMap() {
  if (hasMap)
    assert(munmap(map, size) == 0);
}

FileDescriptorMap::FileDescriptorMap(FileDescriptorMap &&other) noexcept
  : size{other.size}, map{other.map}, hasMap{other.hasMap} {
    other.hasMap = false;
  }

// TODO: not tested
FileDescriptorMap &FileDescriptorMap::operator=(FileDescriptorMap &&other) {
  if (this != &other) {
    if (hasMap)
      assert(munmap(map, size) == 0);
    size = other.size;
    map = other.map;
    hasMap = other.hasMap;
    other.hasMap = false;
  }
  return *this;
}

char *FileDescriptorMap::getMap() { return map; }

size_t FileDescriptorMap::getSize() const { return size; }
