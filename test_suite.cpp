#include "ut.hpp" // Boost's UT!
#include "vec.hpp"

struct Simple {
  int a;
  int b;
};

struct Tracker {
  static int constructor_count;
  static int destructor_count;

  ~Tracker() { ++destructor_count; }

  Tracker() { ++constructor_count; }

  static void reset() { destructor_count = constructor_count = 0; }
};

int Tracker::constructor_count = 0;
int Tracker::destructor_count = 0;

int main() {
  using namespace boost::ut;

  "[constructors]"_test = [] {
    should("empty constructor") = [] {
      SmallVector<int> v;
      expect(0_u == v.size());
    };

    should("count constructor") = [] {
      SmallVector<int> v(10);
      expect(10_u == v.size());
    };

    should("count + value constructor") = [] {
      SmallVector<int> v(10, 5);
      expect(10_u == v.size());
      expect(std::all_of(v.begin(), v.end(), [](int val) { return val == 5; }));
    };
  };

  "[element_access]"_test = [] {
    SmallVector<int> v = {1, 2, 3, 4, 5};

    expect(fatal(v.size() == 5_i));

    should("at()") = [v] {
      expect(v.at(0) == 1_i);
      expect(v.at(4) == 5_i);
      expect(throws([v] { v.at(5); })) << "bad access throws";
    };

    should("operator[]") = [v] {
      expect(v[0] == 1_i);
      expect(v[4] == 5_i);
      // There is no bounds checking here! Let's not try it...
    };

    should("front()/back()/data()") = [v] {
      expect(v.front() == 1_i);
      expect(v.back() == 5_i);
      expect(v.data()[2] == 3_i);
    };
  };

  "[iterators]"_test = [] {
    SmallVector<int> v = {1, 2, 3, 4, 5};

    should("forward") = [v] {
      int cntr = 1;
      for (auto it = v.begin(); it != v.end(); ++it) {
        expect(*it == cntr++);
      }
      cntr = 1;
      for (auto it = v.cbegin(); it != v.cend(); ++it) {
        expect(*it == cntr++);
      }
    };

    should("reverse") = [v] {
      int cntr = 5;
      for (auto it = v.rbegin(); it != v.rend(); ++it) {
        expect(*it == cntr--);
      }
      cntr = 5;
      for (auto it = v.crbegin(); it != v.crend(); ++it) {
        expect(*it == cntr--);
      }
    };
  };

  "[capacity]"_test = [] {
    SmallVector<int, 8> v = {1, 2, 3, 4, 5};

    should("clear()/size()/empty()") = [v] {
      mut(v).clear();
      expect(v.size() == 0_u);
      expect(v.empty());
      // Should also be in array mode
      expect(v.is_array());
      expect(!v.is_vector()); // Obvious from the code, but, let's check!
    };

    should("reserve(count)") = [v] {
      mut(v).resize(10);
      expect(10_u == v.size());
    };

    should("reserve(count, value)") = [v] {
      mut(v).resize(10, 5);
      size_t size = v.size();
      expect(10_u == size);
      for (size_t i = 5; i < size; ++i) {
        expect(v[i] == 5_i);
      }
    };

    should("capacity()") = [v] { expect(v.capacity() == 8_i); };

    should("shrink_to_fit()") = [v] {
      std::vector<int> v_mock = {1, 2, 3, 4, 5};
      // Let's add elements to v (and real vector v_mock) to make it enter
      // vector mode
      for (int i = 0; i < 5; ++i) {
        mut(v).push_back(i);
        v_mock.push_back(i);
      }
      mut(v).shrink_to_fit();
      v_mock.shrink_to_fit();
      // Even if shrink to fit did nothing, both SmallVector and a real vector
      // should do the exact same
      expect(v.size() == v_mock.size())
          << "Expect sizes to be same after shrink_to_fit";
    };
  };

  "[modifiers]"_test = [] {
    should("insert()") = [] {
      SmallVector<Tracker> v(5); // 5 Tracker objects
      // Nicely allow for tracking construct / destruct calls
      Tracker new_val;
      mut(v).insert(v.end(), new_val);
      mut(v).insert(v.end(), std::move(new_val));
      expect(v.size() == 7_i);
    };

    should("emplace()/emplace_back()") = [] {
      SmallVector<Simple> v(5);
      v.emplace(v.end(), 1, 2);
      v.emplace_back(1, 2);
      expect(v.size() == 7_i);
      for (size_t i = 5; i < v.size(); ++i) {
        const auto &s = v[i];
        expect(s.a == 1_i);
        expect(s.b == 2_i);
      }
    };

    should("erase()") = [] {
      SmallVector<Tracker> v(5); // 5 Tracker objects
      Tracker::reset();          // Reset the counts
      v.erase(v.end() - 1);      // Erases the last element
      expect(v.size() == 4_i);
      expect(Tracker::destructor_count == 1_i);
      // We should've had a destruction
      // TODO: FIGURE OUT WHAT'S GOING ON HERE
      Tracker::reset();
      v.erase(v.begin(), v.begin() + 2); // Erase first two
      expect(v.size() == 2_i);
      expect(Tracker::destructor_count == 4_i);
    };
  };
}
