#include <iostream>
#include <string>

int main() {
    std::string s;
//- @s ref StringS
//- StringS.node/kind variable
    s = "hello world";
    std::cout << s << std::endl;
    return 0;
}
