#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include "indentstream.h"
#include "parsefile.h"

namespace proj2 {

// Static reflection would be nice...
std::ostream& operator<< (std::ostream& os, type_t tt) {
    switch (tt) {
        case type_t::t_char              : return os << "char"       ;
        case type_t::t_custom            : return os << "custom"     ;
        case type_t::t_double            : return os << "double"     ;
        case type_t::t_float             : return os << "float"      ;
        case type_t::t_int               : return os << "int"        ;
        case type_t::t_long              : return os << "long"       ;
        case type_t::t_short             : return os << "short"      ;
        case type_t::t_string            : return os << "std::string";
        case type_t::t_void              : return os << "void"       ;
        default                          : return os << "unknown"    ;
    };
}

std::ostream& operator<< (std::ostream& os, container_t ct) {
    switch (ct) {
        case container_t::c_map          : return os << "std::map"   ;
        case container_t::c_tuple        : return os << "std::tuple" ;
        case container_t::c_vector       : return os << "std::vector";
        default                          : return os << "unknown"    ;
    };
}

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
    enum class state { none, header, stubs, boostpython, done };
    state my_state;

    code_generator_base(
        std::string const& path,
        headerfile const& _sourcefile,
        std::unique_ptr<ast> const& _my_ast)
        : ofs(path), ifs(ofs), sourcefile(_sourcefile), my_ast(_my_ast), my_state(state::none) {}

    bool generating_code()           const { return my_state != state::none; }
    bool generating_headers()        const { return my_state == state::header; }
    bool generating_stubs()          const { return my_state == state::stubs; }
    bool generating_boostpython()    const { return my_state == state::boostpython; }
    bool generating_code_finished()  const { return my_state == state::done; }

    virtual ~code_generator_base() = default;
};

class cplusplus_generator : code_generator_base {

// Note: definition here because of forward declaration problems (same issue in tokenizer)
struct cppfile_ast_visitor : ast_visitor_base {
    cplusplus_generator& code_generator;
    cppfile_ast_visitor(cplusplus_generator& _code_generator) : code_generator(_code_generator) {}

    virtual void operator() (ast_basic_variable const&) const override { std::cout << "cpp visiting ast_basic_variable\n"; }
    virtual void operator() (ast_container const&) const override { std::cout << "cpp visiting ast_container\n"; }
    
    virtual void operator() (ast_function const& node) const override {
        std::cout << "cpp visiting ast_function\n"; 
        code_generator.function(node);
    }

    virtual void operator() (ast_include const&) const override { std::cout << "cpp visiting ast_include\n"; }
    
    virtual void operator() (ast_struct const& node) const override {
        std::cout << "cpp visiting ast_struct\n";
        code_generator.struct_(node);
    }
};

    void boostpython_start() {
        ifs << "BOOST_PYTHON_MODULE("
            << sourcefile.modulename
            << "){\n"
            << mpcs::indent
            << "using namespace boost::python;\n\n";
    }

    void boostpython_end() {
        ifs << mpcs::unindent
            << "}\n";
    }

    // void boostpython_indexing(std::string_view custom_type, container_t c_type) {
        // TODO
        // ifs << indexing_suite stuff
    // }

    void basic_variable(ast_basic_variable const& astbv) {
        if (!current_function.empty())
            type_basic(astbv.type);
        else if(!current_struct.empty()) {
            ifs << "\n.def_readwrite(\""
                << astbv.name
                << "\", &"
                << current_struct
                << "::"
                << astbv.name
                << ")";
        }
        if (generating_stubs())
            ifs << astbv.name; 
    }

    void container(ast_container const& astcon) {
        if (!current_function.empty())
            type_container(astcon.type);
        else if(!current_struct.empty()) {
            ifs << "\n.def_readwrite(\""
                << astcon.name
                << "\", &"
                << current_struct
                << "::"
                << astcon.name
                << ")";
        }
        if (generating_stubs())
            ifs << astcon.name;
    }

    void function(ast_function const& astfunc) {
        current_function = astfunc.name;
        if (generating_headers()) {
            type(astfunc.return_type);
            for (auto const& astvar : astfunc.params) {
                variable(astvar);
            }
        } else if (generating_stubs()) {
            type(astfunc.return_type);
            ifs << astfunc.name << '(';

            for (auto astvar = astfunc.params.cbegin(); astvar != astfunc.params.cend(); astvar++) {
                variable(*astvar);
                if (std::next(astvar) != astfunc.params.cend())
                    ifs << ", "; // ostream_joiner would be nice here...
            }
            ifs << "){\n"
                << mpcs::indent
                << "// IMPLEMENTATION\n"
                << mpcs::unindent
                << "}\n\n";
        } else if (generating_boostpython()) {
            ifs << "def(\""
                << astfunc.name
                << "\", "
                << astfunc.name;
            type(astfunc.return_type);
            ifs << ");\n\n";
        }
        current_function = "";
    }

    void header() {
        ifs << R"c++(/*
MPCS 51045 PROJECT 2
AUTO GENERATED C++ FILE
*/

#include <boost/python.hpp>
)c++";
        if (include_map_indexing_suite_hpp)
            ifs << "#include <boost/python/suite/indexing/map_indexing_suite.hpp>\n";
        if (include_vector_indexing_suite_hpp)
            ifs << "#include <boost/python/suite/indexing/vector_indexing_suite.hpp>\n";
        ifs << "\n#include \"" << sourcefile.filename << "\"\n\n";
    }


    void operator_eqls(std::string_view custom_type) {
        ifs << "bool operator==("
            << custom_type
            << " const & lhs, "
            << custom_type
            << " const & rhs) {\n"
            << mpcs::indent
            << "// CHANGE IF NECESSARY\n"
            << "return &lhs == &rhs;\n"
            << mpcs::unindent
            << "}\n\n";
    }

    void struct_(ast_struct const& aststruct) {
        current_struct = aststruct.name;
        if (generating_headers()) {
            // TODO check for containers in members
        } else if (generating_boostpython()) {
            ifs << "class_<"
                << aststruct.name
                << ">(\""
                << aststruct.name
                << "\")"
                << mpcs::indent;
            for (auto const& member : aststruct.members) {
                variable(member);
            }
            ifs << ";\n\n"
                << mpcs::unindent;
        }
        current_struct = "";
    }

    void type(ast_type const& asttype) {
        std::visit(overloaded {
            [this](ast_type_basic     const& tb  ){ type_basic    (tb); },
            [this](ast_type_container const& tcon){ type_container(tcon); }
        }, asttype);
    }

    void type_basic(ast_type_basic const& asttype, container_t c_type = container_t::c_unknown) {
        if (generating_headers()) {
            if (c_type != container_t::c_unknown && asttype.type == type_t::t_custom) {
                comparison_required.insert(asttype.custom_typename);
                // indexing_required.insert({ asttype.custom_typename, c_type});
                if (c_type == container_t::c_map)
                    include_map_indexing_suite_hpp = true;
                else if (c_type == container_t::c_vector)
                    include_vector_indexing_suite_hpp = true;
            }
        } else if (generating_stubs()) {
            if (asttype.mod_unsigned)
                ifs << "unsigned ";
            if (asttype.type == type_t::t_custom) {
                ifs << asttype.custom_typename;
            } else
                ifs << asttype.type;
            ifs << ' ';
            if (asttype.mod_const)
                ifs << "const ";
            if (asttype.mod_ptr)
                ifs << "* ";
            if (asttype.mod_ref)
                ifs << "& ";
        } else if (generating_boostpython()) {
            if (asttype.mod_ptr || asttype.mod_ref)
                ifs << ", return_value_policy<reference_existing_object>()";
        }
    }

    void type_container(ast_type_container const& asttype) {
        if (generating_headers()) {
            for (auto const& typebasic : asttype.template_types) {
                type_basic(typebasic, asttype.type);
            }
        } else if (generating_stubs()) {
            ifs << asttype.type;
            ifs << '<';
            for (auto typebasic = asttype.template_types.cbegin(); typebasic != asttype.template_types.cend(); typebasic++) {
                type_basic(*typebasic);
                if (std::next(typebasic) != asttype.template_types.cend())
                    ifs << ", "; // ostream_joiner would be nice here...
            }
            ifs << "> ";
            if (asttype.mod_const)
                ifs << "const ";
            if (asttype.mod_ptr)
                ifs << "* ";
            if (asttype.mod_ref)
                ifs << "& ";
        } else if (generating_boostpython()) {
            if (asttype.mod_ptr || asttype.mod_ref)
                ifs << ", return_value_policy<reference_existing_object>()";
        }
    }

    // void using_() {

    // }

    void variable(ast_variable const& astvar) {
        std::visit(overloaded {
            [this](ast_basic_variable const& bv ){ basic_variable (bv); },
            [this](ast_container      const& con){ container      (con); }
        }, astvar);

    }

    cppfile_ast_visitor my_ast_visitor;
    std::set<std::string_view>                   comparison_required;
    std::map<std::string_view, std::string_view> indexing_required;
    bool                                         include_map_indexing_suite_hpp;
    bool                                         include_vector_indexing_suite_hpp;
    // bool                                         writing_container;
    std::string_view                             current_function;
    std::string_view                             current_struct;
public:
    cplusplus_generator(headerfile const& source, std::unique_ptr<ast> const& my_ast) : 
        code_generator_base(source.cppfile, source, my_ast),
        my_ast_visitor(*this),
        comparison_required(),
        indexing_required(),
        include_map_indexing_suite_hpp(false),
        include_vector_indexing_suite_hpp(false),
        current_function(),
        current_struct()
        {}

    // first pass
    void generate_header() {
        my_state = state::header;
        for (auto const& node : *my_ast) {
            std::visit(my_ast_visitor, node);
        }
        header();
    }

    // second pass
    void generate_stubs() {
        my_state = state::stubs;
        for (auto const& node : *my_ast) {
            std::visit(my_ast_visitor, node);
        }
        for (std::string_view custom_type : comparison_required) {
            operator_eqls(custom_type);
        }
    }

    // third pass
    void generate_boostpython() {
        my_state = state::boostpython;
        boostpython_start();
        for (auto const& node : *my_ast) {
            std::visit(my_ast_visitor, node);
        }
        for (auto container : indexing_required) {
            // TODO: make this work
            // boostpython_indexing(container.first, container.second);
        }
        boostpython_end();
        my_state = state::done;
    }
};

class python_generator : code_generator_base {

// Note: definition here because of forward declaration problems (same issue above and in tokenizer)
struct pythonfile_ast_visitor : ast_visitor_base {
    python_generator& code_generator;
    pythonfile_ast_visitor(python_generator& _code_generator) : code_generator(_code_generator) {}

    virtual void operator() (ast_basic_variable const&) const override { std::cout << "python visiting ast_basic_variable\n"; }
    virtual void operator() (ast_container      const&) const override { std::cout << "python visiting ast_container\n"; }
    virtual void operator() (ast_function       const&) const override { std::cout << "python visiting ast_function\n"; }
    virtual void operator() (ast_include        const&) const override { std::cout << "python visiting ast_include\n"; }
    
    virtual void operator() (ast_struct const& node) const override {
        std::cout << "python visiting ast_struct\n";
        code_generator.class_(node);
    }
};

    pythonfile_ast_visitor my_ast_visitor;

    void class_(ast_struct const& aststruct) {
        if (generating_stubs()) {
            if (structs_seen.find(aststruct.name) == structs_seen.end()) {
                ifs << "\nnew_"
                    << aststruct.name
                    << " = "
                    << sourcefile.modulename
                    << '.'
                    << aststruct.name
                    << "\nprint( dir("
                    << "new_"
                    << aststruct.name
                    << ") )\n";
                structs_seen.insert(aststruct.name);
            }
        }
    }

    void header() {
        ifs << R"python("""
MPCS 51045 PROJECT 2
AUTO GENERATED PYTHON FILE
"""

)python";
        ifs << "import " << sourcefile.modulename << "\n";
    }

    std::set<std::string_view, std::less<>> structs_seen;
public:
    python_generator(headerfile const& source, std::unique_ptr<ast> const& my_ast) :
        code_generator_base(source.pyfile, source, my_ast),
        my_ast_visitor(*this),
        structs_seen()
        {}

    void generate_header() {
        my_state = state::header;
        header();
    }

    void generate_stubs() {
        my_state = state::stubs;
        for (auto const& node : *my_ast) {
            std::visit(my_ast_visitor, node);
        }
        my_state = state::done;
    }
};

void write_cppfile(headerfile const& sourcefile, std::unique_ptr<ast> const& my_ast) {
    auto cppgen = cplusplus_generator(sourcefile, my_ast);
    cppgen.generate_header();
    cppgen.generate_stubs();
    cppgen.generate_boostpython();
}

void write_pythonfile(headerfile const& sourcefile, std::unique_ptr<ast> const& my_ast) {
    auto pythongen = python_generator(sourcefile, my_ast);
    pythongen.generate_header();
    pythongen.generate_stubs();
}

void cpptopy(std::string_view sourcefile, std::unique_ptr<ast> const& my_ast) {    
    headerfile hfile = parse_headerfile(sourcefile);

    std::thread cpp(write_cppfile,    std::cref(hfile), std::cref(my_ast));
    std::thread py (write_pythonfile, std::cref(hfile), std::cref(my_ast));
    cpp.join();
    py.join();
}

}