#include <iostream>
using namespace std;

void copy_fct() {
  int v1[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int v2[10];
  int *yp = nullptr;
  int &yr = v1[0];

  for (auto i = 0; i < 10; i++) {
    v2[i] = v1[i];
  };

  for (auto &x : v2) {
    x++;
    yp = &x;
    yr = x;
  }

  cout << "yp=" << yp << " *yp=" << *yp << endl;
  cout << "yr=" << yr << " *yr=" << yr << endl;

  cout << "v1: ";
  for (auto x : v1) {
    cout << x << " ";
  }
  cout << endl;

  cout << "v2: ";
  for (auto x : v2) {
    cout << x << " ";
  }
  cout << endl;
}

int main() {
  copy_fct();
  return 0;
}
