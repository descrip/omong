#ifndef BTREE_H
#define BTREE_H

#include "fd.h"

#include <map>
#include <memory>
#include <algorithm>

#ifdef DEBUG
  #include <iostream>
#endif

// TODO what if KeyType isn't default constructible?
// KeyType should be default constructible + okay with copying

template <typename Params>
class BTree {
public:
  using KeyType       = typename Params::KeyType;
  using OffsetType    = typename Params::OffsetType;
  using SizeType      = typename Params::SizeType;

private:
  class BTreeInfo;
  class BNode;
  class BTreeIterator;
  
  FileDescriptor fd_;
  BTreeInfo info_;
  std::map<OffsetType, std::shared_ptr<BNode>> cache_;

public:
  BTree(FileDescriptor &&fd)
    : fd_{std::move(fd)}, info_{fd_.loadMap(0)} {}

  std::shared_ptr<BNode> lowerBound(const KeyType &key,
      bool splitFullBNodes = false);
  void insert(const KeyType &key);
  SizeType erase(const KeyType &key);

#ifdef DEBUG
  void test();
  void dump();
  bool verify(std::shared_ptr<BNode> node);
  bool verify();
#endif

private:
  std::shared_ptr<BNode> getBNode(OffsetType offset);
  std::shared_ptr<BNode> makeBNode(bool isLeaf);
  std::shared_ptr<BNode> loadBNode(OffsetType offset) {
    return std::make_shared<BNode>(*this, offset, fd_.loadMap(offset));
  }

  void splitChild(std::shared_ptr<BNode> parent, SizeType childInd);

  // std::shared_ptr<BNode> eraseLowerBound(const KeyType &key, );

  const BTreeInfo &getInfo() const { return info_; };
};

template <typename Params>
std::shared_ptr<typename BTree<Params>::BNode>
BTree<Params>::lowerBound(const KeyType &key, bool splitFullBNodes) {
  auto root = getBNode(info_.root());
  if (splitFullBNodes && root->isFull()) {
    auto newRoot = makeBNode(false);
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
      // TODO crappy code
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
void
BTree<Params>::insert(const KeyType &key) {
  // TODO return an iterator
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

/*
template <typename Params>
std::shared_ptr<typename BTree<Params>::BNode>
BTree<Params>::eraseLowerBound(const KeyType &key, SizeType &ind) {
  auto curr = getBNode(info_.root());
  while (true) {
    if (curr->isLeaf())
  }
  auto curr = getBNode(key);
  SizeType ind = curr.lowerBound(key);
  if (ind == curr.size())
    return 0;
}
*/

template <typename Params>
typename BTree<Params>::SizeType
BTree<Params>::erase(const KeyType &key) {
  auto curr = getBNode(key);
  SizeType ind = curr.lowerBound(key);
  if (ind == curr.size())
    return 0;
}

template <typename Params>
std::shared_ptr<typename BTree<Params>::BNode>
BTree<Params>::getBNode(OffsetType offset) {
  // from https://channel9.msdn.com/Events/GoingNative/2013/My-Favorite-Cpp-10-Liner
  // TODO not exactly what we need, replace with reaper process
  auto sp = cache_[offset];
  if (!sp)
    cache_[offset] = sp = loadBNode(offset);
  return sp;
}

template <typename Params>
std::shared_ptr<typename BTree<Params>::BNode>
BTree<Params>::makeBNode(bool isLeaf) {
  if (fd_.size() <= info_.nextFree())
    fd_.truncate(info_.nextFree()*2);
  // TODO make a freelist
  auto ret = std::make_shared<BNode>(*this, info_.nextFree(),
      fd_.loadMap(info_.nextFree()));
  cache_[info_.nextFree()] = ret;
  info_.nextFree() += fd_.getPageSize();
  ret->isLeaf() = isLeaf;
  ret->size() = 0;
  return ret;
}

// assumes that this node is not full, the child node at childInd is full
template <typename Params>
void
BTree<Params>::splitChild(std::shared_ptr<typename BTree<Params>::BNode> parent,
    SizeType childInd) {
  auto child = getBNode(parent->children()[childInd]);
  SizeType order = child->getOrder();

  auto newChild = makeBNode(child->isLeaf());
  newChild->size() = order/2;
  child->size() = order/2;
  newChild->follow() = child->follow();
  child->follow() = newChild->getOffset();

  for (SizeType i = 0; i < order/2; ++i)
    newChild->keys()[i] = child->keys()[order/2 + !newChild->isLeaf() + i];
  if (!child->isLeaf())
    for (SizeType i = 0; i <= order/2; ++i)
      newChild->children()[i] = child->children()[order/2 +
          !newChild->isLeaf() + i];

  for (SizeType i = parent->size(); i > childInd; --i)
    parent->keys()[i] = parent->keys()[i-1];
  parent->keys()[childInd] = child->keys()[order/2];
  for (SizeType i = parent->size()+1; i > childInd+1; --i)
    parent->children()[i] = parent->children()[i-1];
  parent->children()[childInd+1] = newChild->getOffset();
  ++parent->size();
}

template <typename Params>
class BTree<Params>::BTreeInfo {
  FileDescriptorMap map_;

  struct Impl {
    OffsetType root;
    OffsetType nextFree;
    SizeType internalOrder;
    SizeType leafOrder;
  } *impl_;

public:
  BTreeInfo(FileDescriptorMap &&map)
    : map_{std::move(map)}, impl_{reinterpret_cast<Impl*>(map_.get())} {}

  OffsetType &root() { return impl_->root; }
  const OffsetType &root() const { return impl_->root; }
  OffsetType &nextFree() { return impl_->nextFree; }
  const OffsetType &nextFree() const { return impl_->nextFree; }

  SizeType &internalOrder() { return impl_->internalOrder; }
  const SizeType &internalOrder() const { return impl_->internalOrder; }
  SizeType &leafOrder() { return impl_->leafOrder; }
  const SizeType &leafOrder() const { return impl_->leafOrder; }
};

template <typename Params>
class BTree<Params>::BNode {
  const BTree<Params> &tree_;
  OffsetType offset_;
  FileDescriptorMap map_;

  struct Impl {
    bool isLeaf;
    SizeType size;
  } *impl_;

public:
  BNode(const BTree<Params> &tree, const OffsetType &offset,
      FileDescriptorMap &&map)
    : tree_{tree}, offset_{offset}, map_{std::move(map)},
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

  OffsetType &follow() {
    return *reinterpret_cast<OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(KeyType)*(getOrder()));
  }
  const OffsetType &follow() const {
    return *reinterpret_cast<const OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(KeyType)*(getOrder()));
  }

  OffsetType *children() {
    return reinterpret_cast<OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(OffsetType) + sizeof(KeyType)*(getOrder()));
  }
  const OffsetType *children() const {
    return reinterpret_cast<const OffsetType*>(map_.get() + sizeof(Impl)
        + sizeof(OffsetType) + sizeof(KeyType)*(getOrder()));
  }

  SizeType getOrder() const {
    return (isLeaf() ? tree_.getInfo().leafOrder()
        : tree_.getInfo().internalOrder());
  }
  OffsetType getOffset() const { return offset_; }

  SizeType lowerBound(KeyType key) const {
    return std::lower_bound(keys(), keys()+size(), key) - keys();
  }
  SizeType upperBound(KeyType key) const {
    return std::upper_bound(keys(), keys()+size(), key) - keys();
  }
  bool isFull() const { return size() == getOrder(); }

#ifdef DEBUG
  void dump();
#endif
};

/*
template <typename Params>
class BTree<Params>::BTreeIterator {
  const BTree<Params> &tree_;
  OffsetType offset_;
  SizeType ind_;
  KeyType key_;

private:
  BTreeIterator(const BTree<Params> &tree, const OffsetType &offset,
      const SizeType &ind)
    : tree_{tree}, offset_{offset}, ind_{ind} { peek(); }

public:
  BTreeIterator &operator++();

private:
  void peek();
  void ensure();
};

template <typename Params>
typename BTree<Params>::BTreeIterator&
BTree<Params>::BTreeIterator::operator++() {
  auto node = tree.getBNode(offset_);
  ++ind_;
  if (ind_ == node.size()) {
    offset_ = node.follow();
    ind_ = 0;
  }
  peek();
  return *this;
}

template <typename Params>
void
BTree<Params>::BTreeIterator::peek() {
  // TODO exception safe?
  auto node = tree.getBNode(offset_);
  key_ = node.keys()[ind_];
}

template <typename Params>
void
BTree<Params>::BTreeIterator::ensure() {
  auto node = tree.getBNode(offset_);
  if (node->keys()[ind_]
}
*/

#ifdef DEBUG
template <typename Params>
void
BTree<Params>::test() {     // TEST
  // INIT CODE
  // TODO move this somewhere else

  info_.root() = fd_.getPageSize();
  info_.nextFree() = fd_.getPageSize()*2;
  info_.internalOrder() = 3;
  info_.leafOrder() = 8;

  auto root = loadBNode(info_.root());
  root->size() = 0;
  root->isLeaf() = true;
  root->follow() = 0;

  // TESTING HERE

  auto bn = loadBNode(info_.root());

  insert(-32);
  insert(99);
  insert(2039);
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
void
BTree<Params>::dump() {
  std::cout << "root: " << info_.root() << '\n';
  std::cout << "nextFree: " << info_.nextFree() << '\n';
  std::cout << '\n';
  for (auto &p : cache_)
    p.second->dump();
}

template <typename Params>
bool
BTree<Params>::verify() {
  if (!verify(getBNode(info_.root())))
    return false;
  return true;
}

template <typename Params>
bool
BTree<Params>::verify(std::shared_ptr<typename BTree<Params>::BNode> node) {
  if (node->getOffset() == info_.root()) {
    if (!(1 <= node->size() && node->size() <= node->getOrder())) {
      std::cout << node->getOffset() << '\n';
      return false;
    }
  } else {
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

template <typename Params>
void
BTree<Params>::BNode::dump() {
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

#endif
