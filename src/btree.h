#ifndef BTREE_H
#define BTREE_H

#include "fd.h"

#include <map>
#include <memory>

#ifdef DEBUG
  #include <iostream>
#endif

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

  // Impl *operator->() const { return impl_; }

  OffsetType &root() { return impl_->root; }
  const OffsetType &root() const { return impl_->root; }
  OffsetType &nextFree() { return impl_->nextFree; }
  const OffsetType &nextFree() const { return impl_->nextFree; }
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

  bool &isLeaf() { return impl_->isLeaf; }
  const bool &isLeaf() const { return impl_->isLeaf; }

  SizeType &size() { return impl_->size; }
  const SizeType &size() const { return impl_->size; }

  KeyType *keys() {
    return reinterpret_cast<KeyType*>(map_.get() + sizeof(Impl));
  }
  const KeyType *keys() const {
    return reinterpret_cast<const KeyType*>(map_.get() + sizeof(Impl));
  }

  OffsetType *children() {
    return reinterpret_cast<OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(KeyType)*(getOrder()));
  }
  const OffsetType *children() const {
    return reinterpret_cast<const OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(KeyType)*(getOrder()));
  }

  SizeType getOrder() const {
    return (impl_->isLeaf ? LeafOrder : InternalOrder);
  }
  OffsetType getOffset() const { return offset_; }

  SizeType lowerBound(KeyType key) const;
  SizeType upperBound(KeyType key) const;
  bool isFull() const { return size() == getOrder(); }

#ifdef DEBUG
  void dump();
#endif
};

template <typename Params>
typename BNode<Params>::SizeType BNode<Params>::lowerBound(KeyType key) const {
  // TODO: linear search for now
  for (SizeType i = 0; i < impl_->size; ++i)
    if (key <= keys()[i])
      return i;
  return impl_->size;
}

template <typename Params>
typename BNode<Params>::SizeType BNode<Params>::upperBound(KeyType key) const {
  // TODO: linear search for now
  for (SizeType i = 0; i < impl_->size; ++i)
    if (key < keys()[i])
      return i;
  return impl_->size;
}

#ifdef DEBUG
template <typename Params>
void BNode<Params>::dump() {
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
#endif

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

#ifdef DEBUG
  void test();
  void dump();
  bool verify(std::shared_ptr<BNode<Params>> node);
  bool verify() { return verify(getBNode(info_.root())); }
#endif

private:
  std::shared_ptr<BNode<Params>> getBNode(OffsetType offset);
  std::shared_ptr<BNode<Params>> makeBNode();
  std::shared_ptr<BNode<Params>> loadBNode(OffsetType offset) {
    return std::make_shared<BNode<Params>>(offset, fd_.loadMap(offset));
  }

  void splitChild(std::shared_ptr<BNode<Params>> parent, SizeType childInd);
};

template <typename Params>
std::shared_ptr<BNode<Params>> BTree<Params>::lowerBound(const KeyType &key, bool splitFullBNodes) {
  auto root = getBNode(info_.root());
  if (splitFullBNodes && root->isFull()) {
    auto newRoot = makeBNode();
    newRoot->children()[0] = root->getOffset();
    info_.root() = newRoot->getOffset();
    splitChild(newRoot, 0);
    root = newRoot;
  }

  auto curr = getBNode(info_.root());
  while (true) {
    if (curr->isLeaf())
      return curr;
    else {
      SizeType ind = curr->lowerBound(key);
      if (ind < curr->size() && curr->keys()[ind] == key)
        ++ind;
      auto child = getBNode(curr->children()[ind]);
      if (splitFullBNodes && child->isFull())
        splitChild(curr, ind);
      ind = curr->lowerBound(key);
      if (ind < curr->size() && curr->keys()[ind] == key)
        ++ind;
      curr = getBNode(curr->children()[ind]);
    }
  }
}

template <typename Params>
void BTree<Params>::insert(const KeyType &key) {
  auto curr = lowerBound(key, true);
  SizeType ind = curr->lowerBound(key);
  if (ind < curr->size() && curr->keys()[ind] == key) {
#ifdef DEBUG
      throw "shouldn't happen?";    // TEST
#endif
    return;
  }
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
  auto ret = std::make_shared<BNode<Params>>(info_.nextFree(), fd_.loadMap(info_.nextFree()));
  cache_[info_.nextFree()] = ret;
  info_.nextFree() += fd_.getPageSize();
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
    newChild->keys()[i] = child->keys()[order/2 + !newChild->isLeaf() + i];
  if (!child->isLeaf())
    for (SizeType i = 0; i <= order/2; ++i)
      newChild->children()[i] = child->children()[order/2 + !newChild->isLeaf() + i];

  for (SizeType i = parent->size(); i > childInd; --i)
    parent->keys()[i] = parent->keys()[i-1];
  parent->keys()[childInd] = child->keys()[order/2];
  for (SizeType i = parent->size()+1; i > childInd+1; --i)
    parent->children()[i] = parent->children()[i-1];
  parent->children()[childInd+1] = newChild->getOffset();
  ++parent->size();
}

#ifdef DEBUG
template <typename Params>
void BTree<Params>::test() {     // TEST
  fd_.truncate(4096*100);

  info_.root() = fd_.getPageSize();
  info_.nextFree() = fd_.getPageSize()*2;

  auto bn = loadBNode(info_.root());

  bn->isLeaf() = true;
  bn->size() = 3;
  bn->keys()[0] = -32;
  bn->keys()[1] = 99;
  bn->keys()[2] = 2039;
  insert(-239);
  insert(-999);
  insert(98);
  insert(90909);
  insert(1024);
  insert(0);
  insert(-3);
  insert(1010111);
  insert(1010113);
  insert(999);
  insert(9998);
  insert(100);
  for (int i = 3000; i <= 3100; ++i)
    insert(i);

  dump();
  assert(verify());
}

template <typename Params>
void BTree<Params>::dump() {
  std::cout << "root: " << info_.root() << '\n';
  std::cout << "nextFree: " << info_.nextFree() << '\n';
  std::cout << '\n';
  for (auto &p : cache_)
    p.second->dump();
}

template <typename Params>
bool BTree<Params>::verify(std::shared_ptr<BNode<Params>> node) {
  if (node->getOffset() == info_.root()) {
    if (!(1 <= node->size() && node->size() <= node->getOrder())) {
      std::cout << node->getOffset() << '\n';
      return false;
    }
  }
  else {
    SizeType ord = node->getOrder();
    if (!(ord/2 <= node->size() && node->size() <= ord)) {
      std::cout << node->getOffset() << '\n';
      return false;
    }
  }
  if (!node->isLeaf())
    for (SizeType i = 0; i <= node->size(); ++i)
      if (!verify(getBNode(node->children()[i]))) {
        std::cout << node->getOffset() << '\n';
        return false;
      }
  return true;
}
#endif

#endif
