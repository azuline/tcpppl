#include <iostream>
using namespace std;

bool accept() {
  int tries = 0;
  while (tries < 3) {
    cout << "Do you want to proceed? [y/n] ";
    char answer = 0;
    cin >> answer;
    switch (answer) {
    case 'y':
      return true;
    case 'x':
      return false;
    default:
      cout << "Invalid character." << endl;
      tries++;
    }
  }
  return false;
}

int main() { return !accept(); }
