#include "fdmap.h"

FileDescriptorMap::FileDescriptorMap(int fd, off_t offset, size_t size)
  : size{size},
    map{static_cast<char *>(mmap(nullptr, size, PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, offset))} {}

FileDescriptorMap::FileDescriptorMap(size_t size, char *map)
  : size{size}, map{map} {}

FileDescriptorMap::~FileDescriptorMap() {
  assert(munmap(map, size) == 0);   // TODO
}

char *FileDescriptorMap::getMap() { return map; }

size_t FileDescriptorMap::getSize() const { return size; }
