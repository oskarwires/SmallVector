
#include "vec.hpp"
#include <iostream>

// Just a nice template to print the SmallVector
template <typename T>
void svout(std::string_view prefix, const SmallVector<T> &vec) {
  bool first{true};
  std::cout << prefix << "[";
  for (const auto &val : vec) {
    std::cout << (first ? first = false, "" : ", ") << val;
  }
  std::cout << "]\n";
}

int main() {
  SmallVector<int> test_vec;

  for (size_t i = 0; i < 10; ++i) {
    test_vec.push_back(i);
  }

  svout<int>("Stack allocated: ", test_vec);

  auto pos = test_vec.cbegin() + 1; // 2nd element
  test_vec.erase(pos);

  pos = test_vec.cbegin(); // 1st element
  auto it = test_vec.erase(pos);
  assert(*it == 2);

  svout<int>("First two removed: ", test_vec);

  pos = test_vec.cend() - 1; // Remove end
  it = test_vec.erase(pos);
  assert(it == test_vec.end());

  svout<int>("Removed last element: ", test_vec);

  auto first = test_vec.cbegin() + 1;
  auto last = test_vec.cbegin() + 3;
  it = test_vec.erase(first, last);
  assert(*it == 5);

  svout<int>("Multi erase: ", test_vec);

  test_vec.pop_back();
  assert(test_vec.back() == 7);

  svout<int>("After pop_back(): ", test_vec);
}
