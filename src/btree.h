#ifndef BTREE_H
#define BTREE_H

#include "fd.h"

#include <iostream> // TEST

template <typename Params>
class BTreeInfo {
  using OffsetType    = typename Params::OffsetType;

  FileDescriptorMap map_;

  struct Impl {
    OffsetType root;
    OffsetType nextFree;
  } *impl_;

public:
  BTreeInfo(FileDescriptorMap &&map)
    : map_{std::move(map)}, impl_{reinterpret_cast<Impl*>(map_.get())} {}

  Impl *operator->() const { return impl_; }
};

template <typename Params>
class BNode {
  using KeyType       = typename Params::KeyType;
  using OffsetType    = typename Params::OffsetType;
  using SizeType      = typename Params::SizeType;

  static const size_t
    InternalMinDeg    = Params::InternalMinDeg,
    InternalOrder     = Params::InternalOrder,
    LeafMinDeg        = Params::LeafMinDeg,
    LeafOrder         = Params::LeafOrder;

  OffsetType offset_;
  FileDescriptorMap map_;

  struct Impl {
    bool isLeaf : 1;
    SizeType size;
  } *impl_;

public:
  BNode(const OffsetType &offset, FileDescriptorMap &&map)
    : offset_{offset},
      map_{std::move(map)},
      impl_{reinterpret_cast<Impl*>(map_.get())} {}

  Impl *operator->() const { return impl_; }

  KeyType *keys() {
    return reinterpret_cast<KeyType*>(map_.get() + sizeof(Impl));
  }

  OffsetType *children() {
    return reinterpret_cast<OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(KeyType)*(getOrder()-1));
  }

  SizeType getOrder() const {
    return (impl_->isLeaf ? LeafOrder : InternalOrder);
  }

  SizeType getMinDeg() const {
    return (impl_->isLeaf ? LeafMinDeg : InternalMinDeg);
  }

  SizeType upperBound(KeyType key);

  OffsetType getOffset() const { return offset_; }
};

template <typename Params>
typename BNode<Params>::SizeType BNode<Params>::upperBound(KeyType key) {
  // TODO: linear search for now
  for (size_t i = 0; i < impl_->size; ++i)
    if (key <= keys()[i])
      return i;
  return impl_->size;
}

template <typename Params>
class BTree {
public:
  using KeyType       = typename Params::KeyType;
  using OffsetType    = typename Params::OffsetType;
  using SizeType      = typename Params::SizeType;

private:
  FileDescriptor fd_;
  BTreeInfo<Params> info_;

public:
  BTree(FileDescriptor &&fd)
    : fd_{std::move(fd)}, info_{fd.loadMap(0)} {}

  BNode<Params> lowerBound(const KeyType &key, bool splitFull = false);
  void insert(const KeyType &key);

  void test();

private:
  BNode<Params> loadBNode(OffsetType offset) {
    return BNode<Params>{offset, fd_.loadMap(offset)};
  }

  BNode<Params> makeBNode();
};

// TODO: only tested for just root, test more after insert
template <typename Params>
BNode<Params> BTree<Params>::lowerBound(const KeyType &key, bool splitFull) {
  OffsetType offset = info_->root;
  while (true) {
    BNode<Params> curr = loadBNode(offset);
    if (curr->isLeaf)
      return curr;
    else {
      SizeType ind = curr.upperBound(key);
      offset = curr.children()[ind];
    }
  }
}

template <typename Params>
void BTree<Params>::insert(const KeyType &key) {

}

template <typename Params>
BNode<Params> BTree<Params>::makeBNode() {
  // TODO truncate file
  // TODO make a freelist
  BNode<Params> ret =
    BNode<Params>{info_->nextFree, fd_.loadMap(info_->nextFree)};
  info_->nextFree += fd_.getPageSize();
  return ret;
}

template <typename Params>
void BTree<Params>::test() {     // TEST
  // fd_.truncate(4096*5);

  /*
  info_->root = fd_.getPageSize();
  info_->nextFree = fd_.getPageSize()*2;
  */

  BNode<Params> bn = loadBNode(info_->root);

  /*
  bn->isLeaf = true;
  bn->size = 3;
  bn.keys()[0] = -32;
  bn.keys()[1] = 99;
  bn.keys()[2] = 2039;
  */

  /*
  std::cout << bn->isLeaf << ' ' << bn->size << '\n';
  for (size_t i = 0; i < bn->size; ++i)
    std::cout << bn.keys()[i] << " \n"[i+1==bn->size];
  */ /*
  auto ind = bn.lowerBound(2040);
  std::cout << ind << '\n';
  */

  // info_->nextFree += fd_.getPageSize()*2;
  // BNode<Params> bn2 = makeBNode();

  BNode<Params> curr = lowerBound(3);
  // std::cout << curr.getOffset() << '\n';

  /*
  // this works, fucking black magic
  std::cout << bn.keys()[0] << '\n';
  curr.keys()[0] = -39;
  std::cout << bn.keys()[0] << '\n';
  */
}

#endif
