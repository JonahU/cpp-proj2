#include <boost/python.hpp>

char const* world() {
    return "hello, world";
}

BOOST_PYTHON_MODULE(hello) {
    using namespace boost::python;
    def("world", world);
}
