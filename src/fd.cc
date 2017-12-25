#ifndef FD_H
#define FD_H

#include "fd.h"

FileDescriptor::FileDescriptor(const std::string &filename)
  : fd{open("test.bin", O_RDWR)} {}

FileDescriptor::~FileDescriptor() { close(fd); }

int FileDescriptor::getFile() const { return fd; }

std::unique_ptr<FileDescriptorMap> FileDescriptor::makeMap(off_t offset) const {
  return std::unique_ptr<FileDescriptorMap>{
      new FileDescriptorMap{getFile(), offset, getPageSize()}};
}

size_t FileDescriptor::getPageSize() const {
  size_t ret = sysconf(_SC_PAGE_SIZE);
  assert(ret == 4096);    // TODO
  return ret;
}

#endif
