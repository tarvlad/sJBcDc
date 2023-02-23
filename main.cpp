#include <iostream>
#include "classFileRead.hpp"

int main() {
    ClassFile clf;
    std::string path("../ArithmeticAlgo.class");
    clf.init(path);

    return 0;
}
