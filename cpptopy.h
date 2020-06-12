#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include "indentstream.h"
#include "parsefile.h"

namespace proj2 {

struct headerfile {
    std::string_view const source;
    std::string      const filename;
    std::string      const extension;
    std::string      const cppfile;
    std::string      const pyfile;
    std::string      const modulename;
};

// Note: not happy about the amount of string copying/ redundancy in this function
headerfile parse_headerfile (std::string_view headerpath) { 
    std::filesystem::path filepath(headerpath);
    std::string filename(filepath.filename());
    size_t dot_index = filename.find_last_of('.');
    std::string no_extension = filename.substr(0, dot_index);
    return { 
        headerpath,
        std::move(filename),
        filepath.extension(),
        filepath.replace_extension(".cpp").relative_path(),
        filepath.replace_extension(".py").relative_path(),
        std::move(no_extension)};
}

class code_generator_base {
    std::ofstream ofs;
protected:
    mpcs::IndentStream ifs;
    headerfile const& sourcefile;
    std::unique_ptr<ast> const& my_ast;
    code_generator_base(std::string const& path, headerfile const& _sourcefile, std::unique_ptr<ast> const& _my_ast)
        : ofs(path), ifs(ofs), sourcefile(_sourcefile), my_ast(_my_ast) {}
};

class cplusplus_generator : code_generator_base {

// Note: definition here because of forward declaration problems (same issue in tokenizer)
struct cppfile_ast_visitor : ast_visitor_base {
    cplusplus_generator const& codegen;
    cppfile_ast_visitor(cplusplus_generator const& _codegen) : codegen(_codegen) {}

    virtual void operator() (ast_basic_variable const&) const override { std::cout << "cpp visiting ast_basic_variable\n"; }
    virtual void operator() (ast_container      const&) const override { std::cout << "cpp visiting ast_container\n"; }
    virtual void operator() (ast_function       const&) const override { std::cout << "cpp visiting ast_function\n"; }
    virtual void operator() (ast_include        const&) const override { std::cout << "cpp visiting ast_include\n"; }
    virtual void operator() (ast_struct         const&) const override { std::cout << "cpp visiting ast_struct\n"; }
};

    cppfile_ast_visitor my_ast_visitor;

    void basic_variable() {

    }

    void boostpython() {
        ifs << R"c++(
BOOST_PYTHON_MODULE(container) {
)c++";
        ifs << mpcs::indent;
        ifs << R"c++(using namespace boost::python;
)c++";
    }

    void function() {

    }

    void header() {
        ifs << R"c++(/*
MPCS 51045 PROJECT 2
AUTO GENERATED C++ FILE
*/

)c++"
            << "#include <boost/python.hpp>\n"
            << "#include <boost/python/suite/indexing/map_indexing_suite.hpp>\n" // TODO: only include map + vector headers if necessary
            << "#include <boost/python/suite/indexing/vector_indexing_suite.hpp>\n\n"
            << "#include \"" << sourcefile.filename << "\"\n\n";
    }


    void operator_eqls() {
        ifs << "bool operator==("
        // TODO: insert basic variable
            << "lhs, "
        // insert basic variable
            << "rhs) {\n"
            << mpcs::indent
            << "// CHANGE IF NECESSARY\n"
            << "return &lhs == &rhs;\n"
            << mpcs::unindent
            << "}\n";
    }

    // void struct_() {

    // }

    // void using_() {

    // }
public:
    cplusplus_generator(headerfile const& source, std::unique_ptr<ast> const& my_ast) : 
        code_generator_base(source.cppfile, source, my_ast),
        my_ast_visitor(*this) {}

    void generate_header() {
        header();
    }

    // first pass
    void generate_function_stubs() {
        for (auto const& node : *my_ast) {
            std::visit(my_ast_visitor, node);
        }
        operator_eqls();
    }

    // second pass
    void generate_boost_python() {
        for (auto const& node : *my_ast) {
            std::visit(my_ast_visitor, node);
        }
        boostpython();
    }
};

class python_generator : code_generator_base {

struct pythonfile_ast_visitor : ast_visitor_base {
    python_generator const& codegen;
    pythonfile_ast_visitor(python_generator const& _codegen) : codegen(_codegen) {}

    virtual void operator() (ast_basic_variable const&) const override { std::cout << "python visiting ast_basic_variable\n"; }
    virtual void operator() (ast_container      const&) const override { std::cout << "python visiting ast_container\n"; }
    virtual void operator() (ast_function       const&) const override { std::cout << "python visiting ast_function\n"; }
    virtual void operator() (ast_include        const&) const override { std::cout << "python visiting ast_include\n"; }
    virtual void operator() (ast_struct         const&) const override { std::cout << "python visiting ast_struct\n"; }
};

    pythonfile_ast_visitor my_ast_visitor;

    void header() {
        ifs << R"python("""
MPCS 51045 PROJECT 2
AUTO GENERATED PYTHON FILE
"""

)python";
        ifs << "import " << sourcefile.modulename << '\n';
    }

public:
    python_generator(headerfile const& source, std::unique_ptr<ast> const& my_ast) :
        code_generator_base(source.pyfile, source, my_ast),
        my_ast_visitor(*this) {}

    void generate_header() {
        header();
    }
};

void write_cppfile(headerfile const& sourcefile, std::unique_ptr<ast> const& my_ast) {
    auto cppgen = cplusplus_generator(sourcefile, my_ast);
    cppgen.generate_header();
    cppgen.generate_function_stubs();
    cppgen.generate_boost_python();

}

void write_pythonfile(headerfile const& sourcefile, std::unique_ptr<ast> const& my_ast) {
    auto pythongen = python_generator(sourcefile, my_ast);
    pythongen.generate_header();
}

void cpptopy(std::string_view sourcefile, std::unique_ptr<ast> const& my_ast) {    
    headerfile hfile = parse_headerfile(sourcefile);

    std::thread cpp(write_cppfile,    std::cref(hfile), std::cref(my_ast));
    std::thread py (write_pythonfile, std::cref(hfile), std::cref(my_ast));
    cpp.join();
    py.join();
}

}