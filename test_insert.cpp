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

  for (size_t i = 0; i < 5; ++i) {
    test_vec.push_back(i);
  }

  svout<int>("Stack allocated: ", test_vec);

  auto pos = test_vec.cbegin() + 1; // 2nd element
  test_vec.insert(pos, 100);

  pos = test_vec.cbegin(); // 1st element
  test_vec.insert(pos, 9);
  assert(test_vec[0] == 9);
  assert(test_vec[2] == 100);

  svout<int>("Stack insert: ", test_vec);
  std::cout << "Is array? " << test_vec.is_array() << "\n";

  for (size_t i = 5; i < 8; ++i) {
    test_vec.push_back(i);
  }

  svout<int>("Vector allocated: ", test_vec);
  std::cout << "Is vector? " << test_vec.is_vector() << "\n";

  pos = test_vec.cbegin(); // 1st element
  test_vec.insert(pos, 1000);
  assert(test_vec[0] == 1000);

  svout<int>("Vector insert: ", test_vec);

  // Let's insert a lvalue
  int value = 101;
  pos = test_vec.cbegin(); // 1st element
  test_vec.insert(pos, value);
  assert(test_vec[0] == 101);

  svout<int>("Vector insert: ", test_vec);
}
