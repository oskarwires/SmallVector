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

  for (int i = 0; i < 5; ++i) {
    test_vec.push_back(i);
  }

  svout<int>("Stack allocated: ", test_vec);

  SmallVector<int> s_vec;
  s_vec.push_back(1);
  s_vec.push_back(2);
  svout<int>("Sacrifical vec: ", s_vec);

  test_vec.append(s_vec);

  svout<int>("Appended vec (still stack): ", test_vec);

  std::cout << "New size: " << test_vec.size() << "\n";
  assert(test_vec.size() == 7);
  assert(test_vec.is_array());

  s_vec.clear();
  for (int i = 10; i < 15; ++i) {
    s_vec.push_back(i);
  }

  svout<int>("New sacrifical vec: ", s_vec);

  test_vec.append(std::move(s_vec));

  svout<int>("Appended vec (now a vector): ", test_vec);

  std::cout << "New size: " << test_vec.size() << "\n";
  assert(test_vec.size() == 12);
  assert(test_vec.is_vector());
}
