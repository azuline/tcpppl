#include <iostream>

int main() {
    int i = 1;
    int &j = i;
    int *k = &i;
    std::cout << "sizeof(i)=" << sizeof(i) << " sizeof(j)=" << sizeof(j) << " sizeof(k)=" << sizeof(k) << std::endl;
}
