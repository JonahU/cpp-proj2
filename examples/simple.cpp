// This a mockup of what the c++ file could look like

/*AUTO GENERATED C++ FILE BY PROJECT2*/

#include <boost/python.hpp>

char const* hello() {
    return "hello, world";
}

BOOST_PYTHON_MODULE(simple) {
    using namespace boost::python;
    def("hello", hello);
}
