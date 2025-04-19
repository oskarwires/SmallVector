#pragma once

#include <cassert>
#include <iterator>
#include <limits>
#include <optional>
#include <vector>

template <class T, size_t STATIC_AMOUNT = 8> class SmallVector {
private:
  using VecT = std::vector<T>;

  // When our arr_ overflows, we move into the internal vec_
  // TODO: Decide, do we move back into vector if unspilled?
  alignas(alignof(T)) std::byte arr_[STATIC_AMOUNT * sizeof(T)];
  std::optional<VecT> vec_;
  size_t size_;

  // Spills over the current array values into a vector, with the ability to
  // reserve more than we need with `extra`
  void spillover(size_t capacity) {
    assert(!vec_); // Should be uninitialised
    assert(capacity >=
           STATIC_AMOUNT); // Should be more than we are moving or same
    vec_ = std::make_optional<VecT>();
    vec_->reserve(capacity);
    for (size_t i = 0; i < size_; i++) {
      vec_->push_back(std::move(get_element(i)));
      get_element_ptr(i)->~T();
    }
  }

  // Just two nice helper functions
  T *get_element_ptr(size_t i) noexcept {
    return reinterpret_cast<T *>(arr_) + i;
  }
  T &get_element(size_t i) noexcept { return *get_element_ptr(i); }

public:
  using iterator = T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  SmallVector() : size_(0) {};

  // ----- ELEMENT ACCESS -----

  T &at(size_t i) {
    if (i >= size_) {
      throw std::out_of_range("Out of range access");
    } else {
      return (*this)[i];
    }
  }

  const T &at(size_t i) const {
    if (i >= size_) {
      throw std::out_of_range("Out of range access");
    } else {
      return (*this)[i];
    }
  }

  T &operator[](size_t i) { return vec_ ? (*vec_)[i] : get_element(i); }
  const T &operator[](size_t i) const {
    return vec_ ? (*vec_)[i] : get_element(i);
  }

  T &front() { return (*this)[0]; }
  const T &front() const { return (*this)[0]; }

  T &back() { return (*this)[size_ - 1]; }
  const T &back() const { return (*this)[size_ - 1]; }

  T *data() noexcept { return vec_ ? &(*vec_)[0] : get_element_ptr(0); }
  const T *data() const noexcept {
    return vec_ ? &(*vec_)[0] : get_element_ptr(0);
  }

  // ----- ITERATORS -----

  iterator begin() noexcept { return vec_ ? &(*vec_)[0] : get_element_ptr(0); }
  const_iterator begin() const noexcept {
    return vec_ ? &(*vec_)[0] : get_element_ptr(0);
  }
  const_iterator cbegin() const noexcept {
    return vec_ ? &(*vec_)[0] : get_element_ptr(0);
  }

  iterator end() noexcept {
    return vec_ ? &(*vec_)[size_] : get_element_ptr(size_);
  }
  const_iterator end() const noexcept {
    return vec_ ? &(*vec_)[size_] : get_element_ptr(size_);
  }
  const_iterator cend() const noexcept {
    return vec_ ? &(*vec_)[size_] : get_element_ptr(size_);
  }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  // ----- CAPACITY -----

  // Returns true if the SmallVector is empty
  bool empty() const { return size_ == 0; }

  // Returns the current amount of stored values T in the SmallVector
  size_t size() const { return size_; }

  // The maximum size is limited by the vector, not the static storage
  static size_t max_size() noexcept { return std::numeric_limits<T>::max(); }

  // Reserves the requested amount if the size is > the current capacity (sum of
  // stack storage & vector's capacity). Has the side effect of initialising the
  // internal vector and reserving size if size > STATIC_SIZE
  void reserve(size_t size) {
    // If we want the amount statically stored, then, do nothing!
    if (size <= STATIC_AMOUNT) {
      return;
    } else {
      if (!vec_) {
        spillover(size);
      } else {
        vec_->reserve(size);
      }
    }
  }

  // Returns the capacity of the SmallVector
  // If the internal vector hasn't been allocated (no spillover), returns the
  // size of the stack allocated array, and if it has, then the vector's
  // capacity
  size_t capacity() const noexcept {
    if (!vec_) {
      return STATIC_AMOUNT;
    } else {
      return vec_->capacity();
    }
  }

  // Has no side effects if using the internal static storage, otherwise calls
  // the vector's `shrink_to_fit()`
  void shrink_to_fit() {
    if (vec_) {
      vec_->shrink_to_fit();
    }
  }

  // ----- MODIFIERS -----

  void clear() {
    if (vec_) {
      // All the static elements have been moved to the vector, and already
      // destructed by `spillover()`, so all we need to do is clear the vec
      vec_->clear();
    } else {
      for (size_t i = size_; i > 0; --i) {
        get_element_ptr(i - 1)->~T();
      }
    }
    size_ = 0;
  }

  // Pushes data to the back of our SmallVector
  void push_back(T &&val) {
    if (!vec_) {
      if (size_ >= STATIC_AMOUNT) {
        spillover(size_ + 1);
        vec_->push_back(std::forward<T>(val));
      } else {
        new (get_element_ptr(size_)) T(std::forward<T>(val));
      }
    } else {
      vec_->push_back(std::forward<T>(val));
    }
    size_++;
  }
  void push_back(const T &val) {
    if (!vec_) {
      if (size_ >= STATIC_AMOUNT) {
        spillover(size_ + 1);
        vec_->push_back(val);
      } else {
        new (get_element_ptr(size_)) T(val);
      }
    } else {
      vec_->push_back(val);
    }
    size_++;
  }
};
