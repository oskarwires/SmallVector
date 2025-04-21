#pragma once

#include <cassert>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

template <class T, size_t STATIC_AMOUNT = 8> class SmallVector {
private:
  using VecT = std::vector<T>;

  // When our arr_ overflows, we move into the internal vec_
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
    for (size_t i = 0; i < size_ && i < STATIC_AMOUNT; ++i) {
      vec_->push_back(std::move(get_element(i)));
      get_element_ptr(i)->~T();
    }
  }

  // Just two nice helper functions
  T *get_element_ptr(size_t i) noexcept {
    return reinterpret_cast<T *>(arr_) + i;
  }
  T &get_element(size_t i) noexcept { return *get_element_ptr(i); }
  const T *get_element_ptr(size_t i) const noexcept {
    return reinterpret_cast<const T *>(arr_) + i;
  }
  const T &get_element(size_t i) const noexcept { return *get_element_ptr(i); }

public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using iterator = T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  SmallVector() : size_(0) {};

  SmallVector(size_t count) : size_(0) {
    reserve(count);
    // Could be in either array/vector mode
    for (size_t i = 0; i < count; ++i) {
      push_back(T());
    }
  }

  SmallVector(size_t count, const T &value) : size_(0) {
    reserve(count);
    for (size_t i = 0; i < count; ++i) {
      push_back(value);
    }
  }

  SmallVector(std::initializer_list<T> init) : size_(0) {
    reserve(init.size());
    std::copy(init.begin(), init.end(), std::back_inserter(*this));
  }

  ~SmallVector() {};

  SmallVector(const SmallVector &other) = default;
  SmallVector(SmallVector &&other) = default;

  SmallVector &operator=(const SmallVector &other) = default;
  SmallVector &operator=(SmallVector &&other) = default;

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

  // Clears the internal static storage, or the vector, whatever's in use
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

  // Inserts `value` at `pos`
  constexpr iterator insert(const_iterator pos, T &&value) {
    const size_t idx = static_cast<size_t>(pos - cbegin());
    size_++;
    if (!vec_ && size_ <= STATIC_AMOUNT) {
      for (size_t i = size_ - 1; i > idx; --i) {
        T *src = get_element_ptr(i - 1);
        T *dst = get_element_ptr(i);
        new (dst) T(std::move(*src));
        src->~T();
      }

      T *dst = get_element_ptr(idx);
      new (dst) T(std::forward<T>(value));
      return dst;
    }

    // If below is true, then, size_ must be > STATIC_AMOUNT
    if (!vec_) {
      spillover(size_);
    }

    // If we spillover, we now have a vector, or we already had a vector
    auto it = vec_->insert(vec_->begin() + idx, std::forward<T>(value));
    // Bit of a nasty way of converting the vector's iterator to ours
    return &(*it);
  }

  // Inserts `value` at `pos`
  constexpr iterator insert(const_iterator pos, const T &value) {
    return insert(pos, T(value));
  }

  // Constructs a new value in-place at `pos` of the SmallVector
  template <typename... Args>
  constexpr iterator emplace(const_iterator pos, Args &&...args) {
    const size_t idx = static_cast<size_t>(pos - cbegin());
    size_++;
    if (!vec_ && size_ <= STATIC_AMOUNT) {
      for (size_t i = size_ - 1; i > idx; --i) {
        T *src = get_element_ptr(i - 1);
        T *dst = get_element_ptr(i);
        new (dst) T(std::move(*src));
        src->~T();
      }

      T *dst = get_element_ptr(idx);
      new (dst) T(std::forward<Args>(args)...);
      return dst;
    }

    // If below is true, then, size_ must be > STATIC_AMOUNT
    if (!vec_) {
      spillover(size_);
    }

    // If we spillover, we now have a vector, or we already had a vector
    auto it = vec_->emplace(vec_->begin() + idx, std::forward<Args>(args)...);
    // Bit of a nasty way of converting the vector's iterator to ours
    return &(*it);
  }

  // Removes the value at the iterator `pos`, and returns the iterator for the
  // value after it
  constexpr iterator erase(const_iterator pos) {
    const size_t idx = static_cast<size_t>(pos - cbegin());
    if (vec_) {
      // Vector case
      auto it = vec_->erase(vec_->begin() + idx);
      size_--;
      return &(*it);
    } else {
      // Array case, let's destruct T at idx
      T *val = get_element_ptr(idx);
      val->~T();
      for (size_t i = idx + 1; i < size_; ++i) {
        T *src = get_element_ptr(i);
        T *dst = get_element_ptr(i - 1);
        new (dst) T(std::move(*src));
        src->~T();
      }
      size_--;
      if (idx == size_) {
        // If the deleted element was the last
        return end();
      } else {
        // Else, just return the element in the spot
        return get_element_ptr(idx);
      }
    }
  }

  // Removes values from the `first` to `last` iterator (but not inclusive of
  // `last`) Returns the iterator to the first value after the last removed
  // value
  constexpr iterator erase(const_iterator first, const_iterator last) {
    const size_t first_idx = static_cast<size_t>(first - cbegin());
    const size_t last_idx = static_cast<size_t>(last - cbegin());
    if (vec_) {
      // Vector case
      auto it =
          vec_->erase(vec_->begin() + first_idx, vec_->begin() + last_idx);
      size_ -= (last_idx - first_idx);
      return &(*it);
    } else {
      // Array case, let's destruct T from first to last
      const size_t diff = last_idx - first_idx;
      for (size_t i = first_idx; i < last_idx; i++) {
        T *val = get_element_ptr(i);
        val->~T();
      }
      for (size_t i = last_idx + 1; i < size_; ++i) {
        T *src = get_element_ptr(i);
        T *dst = get_element_ptr(i - diff);
        new (dst) T(std::move(*src));
        src->~T();
      }
      size_ -= (last_idx - first_idx);
      if (last_idx == size_) {
        // If the deleted element was the last
        return end();
      } else {
        // Else, just return the element in the spot
        return get_element_ptr(last_idx);
      }
    }
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
  // Pushes data to the back of our SmallVector
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

  // Constructs a new value in-place at the back of the SmallVector
  template <typename... Args> void emplace_back(Args &&...args) {
    if (!vec_) {
      if (size_ >= STATIC_AMOUNT) {
        spillover(size_ + 1);
        vec_->emplace_back(std::forward<Args>(args)...);
      } else {
        T *dst = get_element_ptr(size_);
        new (dst) T(std::forward<Args>(args)...);
      }
    } else {
      vec_->emplace_back(std::forward<Args>(args)...);
    }
    size_++;
  }

  // Removes the last element from the SmallVector, if empty this is UB
  constexpr void pop_back() { erase(cend() - 1); }

  constexpr void resize(size_t count, const T &value) {
    if (count == size_)
      return;
    if (count > max_size())
      throw std::length_error("Requested resize above maximum size");

    if (size_ > count) {
      erase(begin() + count, end());
    } else {
      reserve(count);
      for (size_t i = size_; i < count; ++i) {
        push_back(value);
      }
    }
  }

  constexpr void resize(size_t count) {
    if (count == size_)
      return;
    if (count > max_size())
      throw std::length_error("Requested resize above maximum size");

    if (size_ > count) {
      erase(begin() + count, end());
    } else {
      reserve(count);
      for (size_t i = size_; i < count; ++i) {
        emplace_back();
      }
    }
  }

  // ----- NON-MEMBER FUNCTIONS -----
  bool operator==(const SmallVector<T> &other) const {
    if (size_ != other.size())
      return false;
    return std::equal(begin(), end(), other.begin());
  }

  bool operator!=(const SmallVector<T> &other) const {
    return !(*this == other);
  }

  // ----- HELPFUL FUNCTIONS -----
  // These aren't part of the vector / array interface, just nice to have

  // Appends `other` to the end of this SmallVector, clearing `other` after
  void append(SmallVector<T> &&other) {
    // Took until C++23 for std::vector to have something analgous to this
    // (std::append_rage), bit strange
    if (this == &other)
      return;

    const size_t other_size = other.size();
    if (other_size == 0)
      return;

    reserve(size_ + other_size);

    std::move(other.begin(), other.end(), std::back_inserter(*this));
    other.clear();
  }
  // Appends `other` to the end of this SmallVector
  void append(const SmallVector<T> &other) {
    const size_t other_size = other.size();
    if (other_size == 0)
      return;

    reserve(size_ + other_size);
    std::copy(other.begin(), other.end(), std::back_inserter(*this));
  }

  // Returns true if the internal storage is a vector
  // False means it's an array
  bool is_vector() const noexcept { return vec_.has_value(); }

  // Returns true if the internal storage is an array
  // False means it's a vector
  bool is_array() const noexcept { return !is_vector(); }

  // Returns an optional value, containing the internal vector
  // If is_vector() == true, this returns an optional containing the vector
  // If it isn't true, then the optional is empty (!has_value)
  std::optional<VecT> &get_vec() { return vec_; }

  // Returns an optional value, containing the internal vector
  // If is_vector() == true, this returns an optional containing the vector
  // If it isn't true, then the optional is empty (!has_value)
  const std::optional<VecT> &get_vec() const { return vec_; }
};
