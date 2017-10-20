#include <cmath>

template <typename T, typename U>
T wrap(T input, U minimum, U maximum) {
  if (input < minimum) {
    while (input < minimum) {
      input += maximum - minimum;
    }
    return input;
  }
  if (input >= maximum) {
    while (input >= maximum) {
      input -= maximum - minimum;
    }
    return input;
  }
  return input;
}
