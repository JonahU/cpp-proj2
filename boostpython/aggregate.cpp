#include <boost/python.hpp>

#include <iostream>
#include <string>

struct Rocket {
    double      max_speed;
    long        price;
    int         number_of_engines;
    std::string name;
};

Rocket make_rocket_v1() {
    return {
        100.0,
        333222000,
        2,
        "Rocket v1"
    };
}

void launch_rocket_v1(Rocket& r, char const* where, char const* when) {
    std::cout
        << "c++ launching rocket " << r.name << '\n'
        << "speed = " <<  r.max_speed << "mph\n"
        << "location = " << where << '\n'
        << "time = " << when << '\n';
}

BOOST_PYTHON_MODULE(aggregate) {
    using namespace boost::python;

    class_<Rocket>("Rocket")
        .def_readwrite("max_speed", &Rocket::max_speed)
        .def_readwrite("price", &Rocket::price)
        .def_readwrite("number_of_engines", &Rocket::number_of_engines)
        .def_readwrite("name", &Rocket::name);

    def("make_rocket_v1", make_rocket_v1);

    def("launch_rocket_v1", launch_rocket_v1);
}
