#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <variant>
#include <functional>
#include <fstream>

#include "ast.h"
#include "parser.h"

int main() {
    const auto expr = std::make_shared<expression>("a");    

    // test();
    parse("code.txt");

    return 0;
}