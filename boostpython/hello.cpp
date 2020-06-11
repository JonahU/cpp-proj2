// This a mockup of what the c++ file could look like

/*AUTO GENERATED C++ FILE BY PROJECT2*/

#include <boost/python.hpp>

char const* world() {
    return "hello, world";
}

BOOST_PYTHON_MODULE(hello) {
    using namespace boost::python;
    def("world", world);
}
