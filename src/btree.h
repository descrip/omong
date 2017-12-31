#ifndef BTREE_H
#define BTREE_H

#include "fd.h"

#include <map>
#include <memory>
#include <iostream> // TEST

// TODO what if KeyType isn't default constructible?

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

  static const SizeType
    InternalOrder     = Params::InternalOrder,
    LeafOrder         = Params::LeafOrder;

  OffsetType offset_;
  FileDescriptorMap map_;

  struct Impl {
    bool isLeaf;
    SizeType size;
  } *impl_;

public:
  BNode(const OffsetType &offset, FileDescriptorMap &&map)
    : offset_{offset},
      map_{std::move(map)},
      impl_{reinterpret_cast<Impl*>(map_.get())} {}

  BNode(BNode &&other) = default;
  BNode &operator=(BNode &&other) = default;

  auto &isLeaf() { return impl_->isLeaf; }

  SizeType &size() { return impl_->size; }

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

  SizeType lowerBound(KeyType key);
  SizeType upperBound(KeyType key);

  OffsetType getOffset() const { return offset_; }

  bool isFull() const { return impl_->size == getOrder(); }

  void dump() {   // TEST
    std::cout << "offset_: " << getOffset() << '\n';
    std::cout << "isLeaf: " << isLeaf() << '\n';
    std::cout << "size: " << size() << '\n';
    for (SizeType i = 0; i < size(); ++i)
      std::cout << keys()[i] << " \n"[i+1 == size()];
    if (!isLeaf())
      for (SizeType i = 0; i <= size(); ++i)
        std::cout << children()[i] << " \n"[i == size()];
    std::cout << '\n';
  }
};

template <typename Params>
typename BNode<Params>::SizeType BNode<Params>::lowerBound(KeyType key) {
  // TODO: linear search for now
  for (SizeType i = 0; i < impl_->size; ++i)
    if (key <= keys()[i])
      return i;
  return impl_->size;
}

template <typename Params>
typename BNode<Params>::SizeType BNode<Params>::upperBound(KeyType key) {
  // TODO: linear search for now
  for (SizeType i = 0; i < impl_->size; ++i)
    if (key < keys()[i])
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
  std::map<OffsetType, std::shared_ptr<BNode<Params>>> cache_;

public:
  BTree(FileDescriptor &&fd)
    : fd_{std::move(fd)}, info_{fd_.loadMap(0)} {}

  std::shared_ptr<BNode<Params>> lowerBound(const KeyType &key, bool splitFullBNodes = false);
  void insert(const KeyType &key);

  void test();    // TEST

  void dump() {
    std::cout << "root: " << info_->root << '\n';
    std::cout << "nextFree: " << info_->nextFree << '\n';
    std::cout << '\n';
    for (auto &p : cache_)
      p.second->dump();
  }

private:
  std::shared_ptr<BNode<Params>> getBNode(OffsetType offset);

  std::shared_ptr<BNode<Params>> loadBNode(OffsetType offset) {
    return std::make_shared<BNode<Params>>(offset, fd_.loadMap(offset));
  }

  std::shared_ptr<BNode<Params>> makeBNode();

  void splitChild(std::shared_ptr<BNode<Params>> parent, SizeType childInd);
};

// TODO: only tested for just root, test more after insert
template <typename Params>
std::shared_ptr<BNode<Params>> BTree<Params>::lowerBound(const KeyType &key, bool splitFullBNodes) {
  auto root = getBNode(info_->root);
  if (splitFullBNodes && root->isFull()) {
    auto newRoot = makeBNode();
    newRoot->children()[0] = root->getOffset();
    info_->root = newRoot->getOffset();
    splitChild(newRoot, 0);
    root = newRoot;
  }

  auto curr = getBNode(info_->root);
  while (true) {
    if (curr->isLeaf())
      return curr;
    else {
      SizeType ind = curr->upperBound(key);

      if (key == -3)
        std::cout << ind << '\n';

      auto child = getBNode(curr->children()[ind]);
      if (splitFullBNodes && child->isFull()) {
        splitChild(curr, ind);
        curr = getBNode(curr->children()[curr->upperBound(key)]);
      }
      else curr = child;
    }
  }
}

template <typename Params>
void BTree<Params>::insert(const KeyType &key) {
  auto curr = lowerBound(key, true);
  SizeType ind = curr->lowerBound(key);
  if (curr->keys()[ind] == key)
    return;
  for (SizeType i = curr->size(); i > ind; --i)
    curr->keys()[i] = curr->keys()[i-1];
  curr->keys()[ind] = key;
  ++curr->size();
}

template <typename Params>
std::shared_ptr<BNode<Params>> BTree<Params>::getBNode(OffsetType offset) {
  // from https://channel9.msdn.com/Events/GoingNative/2013/My-Favorite-Cpp-10-Liner
  // TODO not exactly what we need, replace with reaper process
  auto sp = cache_[offset];
  if (!sp)
    cache_[offset] = sp = loadBNode(offset);
  return sp;
}

template <typename Params>
std::shared_ptr<BNode<Params>> BTree<Params>::makeBNode() {
  // TODO truncate file
  // TODO make a freelist
  auto ret = std::make_shared<BNode<Params>>(info_->nextFree, fd_.loadMap(info_->nextFree));
  cache_[info_->nextFree] = ret;
  info_->nextFree += fd_.getPageSize();
  ret->isLeaf() = false;
  ret->size() = 0;
  return ret;
}

/* assumes that this node is not full,
 * the child node at childInd is full */
template <typename Params>
void BTree<Params>::splitChild(std::shared_ptr<BNode<Params>> parent, SizeType childInd) {
  auto child = getBNode(parent->children()[childInd]);
  SizeType order = child->getOrder();

  auto newChild = makeBNode();
  newChild->isLeaf() = child->isLeaf();
  newChild->size() = order/2;
  child->size() = order/2;

  for (SizeType i = 0; i < order/2; ++i)
    newChild->keys()[i] = child->keys()[order/2+i];
  if (!child->isLeaf())
    for (SizeType i = 0; i < order/2; ++i)
      newChild->keys()[i] = child->keys()[order/2+i];

  for (SizeType i = parent->size(); i > childInd+1; --i)
    parent->keys()[i] = parent->keys()[i-1];
  parent->keys()[childInd] = child->keys()[order/2];
  for (SizeType i = parent->size(); i >= childInd+1; --i)
    parent->children()[i+1] = parent->children()[i];
  parent->children()[childInd+1] = newChild->getOffset();
  ++parent->size();

  // TODO: probably broken if child isn't leaf
}

template <typename Params>
void BTree<Params>::test() {     // TEST
  // fd_.truncate(4096*5);

  info_->root = fd_.getPageSize();
  info_->nextFree = fd_.getPageSize()*2;

  auto bn = loadBNode(info_->root);

  bn->isLeaf() = true;
  bn->size() = 3;
  bn->keys()[0] = -32;
  bn->keys()[1] = 99;
  bn->keys()[2] = 2039;
  insert(-239);
  insert(-999);

  dump();
  /*
  std::cout << bn->isLeaf() << ' ' << bn->size() << '\n';
  for (size_t i = 0; i < bn->size(); ++i)
    std::cout << bn->keys()[i] << " \n"[i+1==bn->size()];
    */
  /*
  auto ind = bn.lowerBound(2040);
  std::cout << ind << '\n';
  */

  // info_->nextFree += fd_.getPageSize()*2;
  // BNode<Params> bn2 = makeBNode();

  auto curr = lowerBound(-3);
  std::cout << curr->getOffset() << '\n';

  /*
  // this works, fucking black magic
  std::cout << bn.keys()[0] << '\n';
  curr.keys()[0] = -39;
  std::cout << bn.keys()[0] << '\n';
  */
}

#endif
