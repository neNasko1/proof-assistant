#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <variant>
#include <functional>
#include <fstream>

#include "ast.h"
#include "parser.h"

int main(int argc, char **argv) {
    const auto expr = std::make_shared<expression>("a");    

    if(argc < 2) {
        parse("code.txt");
    } else {
        parse(argv[1]);        
    }

    return 0;
}