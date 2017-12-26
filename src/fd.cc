#ifndef FD_H
#define FD_H

#include "fd.h"

FileDescriptor::FileDescriptor(const std::string &filename)
  : fd{open("test.bin", O_RDWR)}, hasFd{true} {}

FileDescriptor::~FileDescriptor() {
  if (hasFd)
    assert(close(fd) == 0);
}

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
  : fd{other.fd}, hasFd{other.hasFd} { other.hasFd = false; }

// TODO: not tested
FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) {
  if (this != &other) {
    if (hasFd)
      assert(close(fd) == 0);
    fd = other.fd;
    hasFd = other.hasFd;
    other.hasFd = false;
  }
  return *this;
}

int FileDescriptor::getFile() { return fd; }

FileDescriptorMap FileDescriptor::makeMap(off_t offset) {
  return FileDescriptorMap{getFile(), offset, getPageSize()};
}

size_t FileDescriptor::getPageSize() {
  size_t ret = sysconf(_SC_PAGE_SIZE);
  assert(ret == 4096);    // TODO testing
  return ret;
}

#endif
