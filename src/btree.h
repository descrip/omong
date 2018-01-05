#ifndef BTREE_H
#define BTREE_H

#include "fd.h"

#include <map>
#include <memory>
#include <algorithm>

#ifdef DEBUG
  #include <iostream>
  #include <cstdlib>
  #include <ctime>
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

  std::shared_ptr<BNode> lowerBound(const KeyType &key);
  void insert(const KeyType &key);
  SizeType erase(const KeyType &key);

#ifdef DEBUG
  void init();
  void test();
  void randomTest();
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

  // method helpers
  SizeType getNextChild(std::shared_ptr<BNode> curr, const KeyType &key) const;
  void splitChild(std::shared_ptr<BNode> parent, SizeType childInd);
  SizeType eraseFromLeaf(std::shared_ptr<BNode> leaf, const KeyType &key);
  SizeType erase(std::shared_ptr<BNode> parent, std::shared_ptr<BNode> curr,
      const KeyType &key);

  const BTreeInfo &getInfo() const { return info_; };
};

template <typename Params>
typename BTree<Params>::SizeType
BTree<Params>::getNextChild(std::shared_ptr<BNode> curr,
    const KeyType &key) const {
  SizeType ind = curr->lowerBound(key);
  if (ind < curr->size() && curr->keys()[ind] == key)
    ++ind;
  return ind;
}

template <typename Params>
std::shared_ptr<typename BTree<Params>::BNode>
BTree<Params>::lowerBound(const KeyType &key) {
  auto curr = getBNode(info_.root());
  while (true)
    if (curr->isLeaf())
      return curr;
    else {
      SizeType ind = getNextChild(curr, key);
      curr = getBNode(curr->children()[ind]);
    }
}

template <typename Params>
void
BTree<Params>::insert(const KeyType &key) {
  // TODO return an iterator

  // if root is full, add a new bnode and make that the new root
  auto root = getBNode(info_.root());
  if (root->isFull()) {
    auto newRoot = makeBNode(false);
    newRoot->children()[0] = root->getOffset();
    info_.root() = newRoot->getOffset();
    splitChild(newRoot, 0);
    root = newRoot;
  }

  auto curr = root;
  while (true)
    if (curr->isLeaf())
      break;
    else {
      SizeType ind = getNextChild(curr, key);
      auto child = getBNode(curr->children()[ind]);
      // if child is full, split it and retry
      if (child->isFull()) {
        splitChild(curr, ind);
        ind = getNextChild(curr, key);
        curr = getBNode(curr->children()[ind]);
      }
      else curr = child;
    }

  SizeType ind = curr->lowerBound(key);
  if (ind < curr->size() && curr->keys()[ind] == key) {
#ifdef DEBUG
      throw "shouldn't happen?";    // TEST
#endif
    return;
  }

  curr->insert(ind, key);
}

template <typename Params>
typename BTree<Params>::SizeType
BTree<Params>::eraseFromLeaf(std::shared_ptr<BNode> leaf, const KeyType &key) {
  SizeType ind = leaf->lowerBound(key);
  if (ind < leaf->size() && leaf->keys()[ind] == key) {
    for (SizeType i = ind; i < leaf->size()-1; ++i)
      leaf->keys()[i] = leaf->keys()[i+1];
    --leaf->size();
    return 1;
  } else return 0;
}

template <typename Params>
typename BTree<Params>::SizeType
BTree<Params>::erase(std::shared_ptr<BNode> parent,
    std::shared_ptr<BNode> curr, const KeyType &key) {

  if (curr.isLeaf())
    return eraseFromLeaf(curr, key);
  SizeType ind = getNextChild(curr, key);
  erase(curr, getBNode(curr->children()[ind]), key);
  SizeType order = curr->getOrder();

  if (curr.size() < order/2) {
    // try borrowing from left sibling
    if (0 < ind) {
      auto sib = getBNode(curr->children()[ind-1]);
      if (sib->size() > order/2) {
        curr->insert(0, sib->keys()[sib->size()-1]);
        sib->erase(sib->size()-1);
        return 1;
      }
    }

    // try borrowing from right sibling
    if (ind+1 < curr->size()) {
      auto sib = getBNode(curr->children()[ind+1]);
      if (sib->size() > order/2) {
        curr->insert(curr->size(), sib->keys()[0]);
        sib->erase(0);
        return 1;
      }
    }

    // try merging with left sibling
    if (0 < ind) {
      auto sib = getBNode(curr->children()[ind-1]);
      // sib->size() == order/2
      for (SizeType i = curr.size(); i >= 1; --i)
        curr->keys()[i] = curr->keys()[i-1];
      curr->keys()[0] = sib->keys()[sib->size()-1];
      --sib->size();
      ++curr->size();
      return 1;
    }
  }
}

/*
template <typename Params>
typename BTree<Params>::SizeType
BTree<Params>::erase(const KeyType &key) {
  auto curr = getBNode(info_.root());
  // edge case: btree is just one node

  while (true) {
    SizeType ind = curr->lowerBound(key);
    if (ind < curr->size() && curr->keys()[ind] == key)
      ++ind;
    auto child = getBNode(curr->children()[ind]);
    if (child.isLeaf()) {
      SizeType ret = eraseFromLeaf(child, key);
      if (ret == 0)
        return 0;
      SizeType order = curr->getOrder();

      if (child->size() < order/2) {


        // try merging with left sibling
        if (0 < ind) {
          auto sib = getBNode(curr->children()[ind-1]);
          // sib->size() == order/2
          for (SizeType i = curr.size(); i >= 1; --i)
            curr->keys()[i] = curr->keys()[i-1];
          curr->keys()[0] = sib->keys()[sib->size()-1];
          --sib->size();
          ++curr->size();
          return 1;
        }
        
      }
  }
}
*/

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
  newChild->follow() = child->follow();
  child->follow() = newChild->getOffset();

  // split
  for (SizeType i = 0; i < order/2; ++i)
    newChild->keys()[i] = child->keys()[order/2 + !newChild->isLeaf() + i];
  if (!child->isLeaf())
    for (SizeType i = 0; i <= order/2; ++i)
      newChild->children()[i] = child->children()[order/2 +
          !newChild->isLeaf() + i];
  child->size() = newChild->size() = order/2;

  parent->insert(childInd, child->keys()[order/2], newChild->getOffset());
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
  bool isFull() const { return size() == getOrder(); }

  SizeType lowerBound(KeyType key) const {
    return std::lower_bound(keys(), keys()+size(), key) - keys();
  }
  SizeType upperBound(KeyType key) const {
    return std::upper_bound(keys(), keys()+size(), key) - keys();
  }

  // for leaf nodes
  void insert(SizeType ind, const KeyType &key);
  // for internal nodes
  void insert(SizeType ind, const KeyType &key, const OffsetType &child);
  void erase(SizeType ind);
  void merge(std::shared_ptr<BNode> other);

#ifdef DEBUG
  void dump();
#endif
};

template <typename Params>
void
BTree<Params>::BNode::insert(SizeType ind, const KeyType &key) {
  std::move_backward(keys()+ind, keys()+size(), keys()+size()+1);
  keys()[ind] = key;
  ++size();
}

template <typename Params>
void
BTree<Params>::BNode::insert(SizeType ind, const KeyType &key,
    const OffsetType &child) {
  insert(ind, key);
  std::move_backward(children()+ind+1, children()+size()+1,
      children()+size()+2);
  children()[ind+1] = child;
}

template <typename Params>
void
BTree<Params>::BNode::erase(SizeType ind) {
  std::move(keys()+ind+1, keys()+size(), keys()+ind);
  if (!isLeaf())
    std::move(children()+ind+2, children()+size(), children()+ind+1);
  --size();
}

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
BTree<Params>::init() {
  info_.root() = fd_.getPageSize();
  info_.nextFree() = fd_.getPageSize()*2;
  info_.internalOrder() = 3;
  info_.leafOrder() = 8;

  auto root = loadBNode(info_.root());
  root->size() = 0;
  root->isLeaf() = true;
  root->follow() = 0;
}

template <typename Params>
void
BTree<Params>::randomTest() {
  init();
  srand(time(nullptr));
  for (int i = 0; i < 56; ++i)
    insert(rand());
  dump();
  assert(verify());
}

template <typename Params>
void
BTree<Params>::test() {
  init();

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
    SizeType order = node->getOrder();
    if (!(order/2 <= node->size() && node->size() <= order)) {
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
