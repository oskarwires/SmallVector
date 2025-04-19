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

  size_t vec_size = my_vec.size();

  for (size_t i = 0; i < vec_size; i++) {
    std::cout << my_vec[i] << " ";
  }

  std::cout << "\n";

  for (size_t i = 0; i < vec_size; i++) {
    std::cout << my_vec.at(i) << " ";
  }

  std::cout << "\n";

  assert(my_vec.front() == 0);
  assert(my_vec.back() == 9);
  assert(my_vec.data()[2] == 2);

  my_vec.clear();
  assert(my_vec.size() == 0);

  std::cout << "Max size of the SmallVector is " << my_vec.max_size() << "\n";

  return 0;
}
