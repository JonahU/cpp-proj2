#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include "parsefile.h"

namespace proj2 {

struct headerfile {
    std::string_view source;
    std::string      filename;
    std::string      extension;
};

headerfile split_headerfile (std::string_view sv) { 
    auto [ _, filename, extension ] = ctre::match<headerfile_regex>(sv);
    return { sv, filename.to_string(), extension.to_string() };
}

struct cpptopy_ast_visitor : ast_visitor_base {
    virtual void operator() (ast_basic_variable const&) const override { std::cout << "cpptopy visiting ast_basic_variable\n"; }
    virtual void operator() (ast_container      const&) const override { std::cout << "cpptopy visiting ast_container\n"; }
    virtual void operator() (ast_function       const&) const override { std::cout << "cpptopy visiting ast_function\n"; }
    virtual void operator() (ast_include        const&) const override { std::cout << "cpptopy visiting ast_include\n"; }
    virtual void operator() (ast_struct         const&) const override { std::cout << "cpptopy visiting ast_struct\n"; }
};

void cpptopy(std::string_view sourcefile, std::unique_ptr<ast> const& my_ast) {    
    headerfile hfile = split_headerfile(sourcefile);

    std::string file_py  = hfile.filename + ".py";
    std::string file_cpp = hfile.filename + ".cpp";

    std::ofstream ofs_py(file_py);
    std::ofstream ofs_cpp(file_cpp);

    ofs_py << "\"\"\"AUTO GENERATED PYTHON FILE BY PROJECT2\"\"\"";
    ofs_cpp << "/*AUTO GENERATED C++ FILE BY PROJECT2*/";

    cpptopy_ast_visitor my_ast_visitor;
    for (auto const& node : *my_ast) {
        std::visit(my_ast_visitor, node);
    }
}

}