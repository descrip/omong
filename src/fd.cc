#ifndef FD_H
#define FD_H

#include "fd.h"

FileDescriptorMap::FileDescriptorMap(int fd, off_t offset, size_t size)
  : size_{size},
    map_{static_cast<char *>(mmap(nullptr, size_, PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, offset))},
    hasMap_{true} {}

FileDescriptorMap::FileDescriptorMap(size_t size, char *map)
  : size_{size}, map_{map}, hasMap_{true} {}

FileDescriptorMap::~FileDescriptorMap() {
  if (hasMap_)
    assert(munmap(map_, size_) == 0);
}

FileDescriptorMap::FileDescriptorMap(FileDescriptorMap &&other) noexcept
  : size_{other.size_}, map_{other.map_}, hasMap_{other.hasMap_} {
    other.hasMap_ = false;
  }

// TODO: not tested
FileDescriptorMap &FileDescriptorMap::operator=(FileDescriptorMap &&other) {
  if (this != &other) {
    if (hasMap_)
      assert(munmap(map_, size_) == 0);
    size_ = other.size_;
    map_ = other.map_;
    hasMap_ = other.hasMap_;
    other.hasMap_ = false;
  }
  return *this;
}

char *FileDescriptorMap::get() { return map_; }
const char *FileDescriptorMap::get() const { return map_; }

size_t FileDescriptorMap::size() const { return size_; }


FileDescriptor::FileDescriptor(const std::string &filename)
  : fd_{open("test.bin", O_RDWR)}, hasFd_{true} {}

FileDescriptor::~FileDescriptor() {
  if (hasFd_)
    assert(close(fd_) == 0);
}

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
  : fd_{other.fd_}, hasFd_{other.hasFd_} { other.hasFd_ = false; }

// TODO: not tested
FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) {
  if (this != &other) {
    if (hasFd_)
      assert(close(fd_) == 0);
    fd_ = other.fd_;
    hasFd_ = other.hasFd_;
    other.hasFd_ = false;
  }
  return *this;
}

int FileDescriptor::fd() const { return fd_; }

FileDescriptorMap FileDescriptor::loadMap(off_t offset) const {
  return FileDescriptorMap{fd(), offset, getPageSize()};
}

size_t FileDescriptor::getPageSize() const {
  size_t ret = sysconf(_SC_PAGE_SIZE);
  assert(ret == 4096);    // TODO testing
  return ret;
}

void FileDescriptor::truncate(off_t size) {
  assert(ftruncate(fd_, size) == 0);   // TODO
}

// TODO untested
off_t FileDescriptor::size() {
  getStats();
  return stats_.st_size;
}

void FileDescriptor::getStats() {
  assert(fstat(fd_, &stats_) == 0);
}

#endif
