#include <complex>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  auto d1 = 2.2;
  auto d2{2.2};
  auto i = 7;
  d1 = d1 + i;
  i = d1 * i;
  cout << "d1=" << d1 << " i=" << i;

  complex<double> z1 = 1;
  complex<double> z2{d1, d2};
  complex<double> z3 = {1, 2};

  vector<int> v{1, 2, 3, 4, 5, 6};

  return 0;
}
