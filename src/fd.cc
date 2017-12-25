#ifndef FD_H
#define FD_H

#include "fd.h"

FileDescriptor::FileDescriptor(const std::string &filename)
  : fd{open("test.bin", O_RDWR)} {}

FileDescriptor::~FileDescriptor() { assert(close(fd) == 0); }

int FileDescriptor::getFile() const { return fd; }

std::unique_ptr<FileDescriptorMap> FileDescriptor::makeMap(off_t offset) const {
  return std::make_unique<FileDescriptorMap>(getFile(), offset, getPageSize());
}

size_t FileDescriptor::getPageSize() const {
  size_t ret = sysconf(_SC_PAGE_SIZE);
  assert(ret == 4096);    // TODO testing
  return ret;
}

#endif
