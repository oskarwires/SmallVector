#include "vec.hpp"
#include <iostream>

int main() {
  SmallVector<int> my_vec;

  assert(my_vec.size() == 0);

  for (size_t i = 0; i < 10; i++) {
    my_vec.push_back(i);
    std::cout << "pushed back " << i << std::endl;
    std::cout << "new size: " << my_vec.size() << std::endl;
    std::cout << "new capacity: " << my_vec.capacity() << std::endl;
    std::cout << std::endl;
  }

  my_vec.shrink_to_fit();
  std::cout << "new capacity after shrink: " << my_vec.capacity() << "\n";
  std::cout << "\n";

  size_t vec_size = my_vec.size();

  // Read with operator[]
  for (size_t i = 0; i < vec_size; i++) {
    std::cout << my_vec[i] << " ";
  }
  std::cout << "\n";

  // Read with .at()
  for (size_t i = 0; i < vec_size; i++) {
    std::cout << my_vec.at(i) << " ";
  }
  std::cout << "\n";

  // Read with forward iterator
  auto end = my_vec.end();
  for (auto it = my_vec.begin(); it != end; ++it) {
    std::cout << *it << " ";
  }
  std::cout << "\n";

  // Read with reverse iterator
  auto rend = my_vec.rend();
  for (auto it = my_vec.rbegin(); it != rend; ++it) {
    std::cout << *it << " ";
  }
  std::cout << "\n";

  assert(my_vec.front() == 0);
  assert(my_vec.back() == 9);
  assert(my_vec.data()[2] == 2);
  assert(*my_vec.begin() == 0);
  assert(*(my_vec.end() - 1) == 9);

  my_vec.clear();
  assert(my_vec.size() == 0);

  std::cout << "Max size of the SmallVector is " << my_vec.max_size() << "\n";

  return 0;
}
