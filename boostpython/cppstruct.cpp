#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> // convert std vector -> py obj
#include <boost/python/suite/indexing/map_indexing_suite.hpp> // convert std map -> py obj

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <tuple>

struct Rocket {
    double      max_speed;
    long        price;
    // int const   number_of_engines; // const = readonly
    int         number_of_engines;
    std::string name;

    // vector conversion boilerplate
    bool operator==(Rocket const& rhs) {
        return this == &rhs;
    }
};

Rocket make_rocket_v1() {
    return {
        100.0,
        333222000,
        2,
        "Rocket v1"
    };
}

void launch_rocket_v1(Rocket& r) {
    std::cout << "launching rocket " << r.name << '\n';
}

std::vector<Rocket> vector_to_python() {
    std::vector<Rocket> rockets = {
        { 100.0, 333222000, 2, "Rocket v1" },
        { 200.0, 444222000, 4, "Rocket v2" },
        { 300.0, 555222000, 8, "Rocket v3" }
    };
    return rockets;
}

std::vector<Rocket>* vector_star_to_python() {
    std::vector<Rocket>* rockets_ptr = new std::vector<Rocket>{
        { 100.0, 333222000, 2, "Rocket v1" },
        { 200.0, 444222000, 4, "Rocket v2" },
        { 300.0, 555222000, 8, "Rocket v3" }
    };
    return rockets_ptr;
}

std::map<std::string,Rocket> map_to_python() {
    std::map<std::string, Rocket> rockets = {
        { "apollo11", { 100.0, 333222000, 2, "Rocket v1" } },
        { "apollo12", { 200.0, 444222000, 4, "Rocket v2" } },
        { "apollo13", { 300.0, 555222000, 8, "Rocket v3" } }
    };
    return rockets;
}

BOOST_PYTHON_MODULE(cppstruct) {
    using namespace boost::python;

    class_<Rocket>("Rocket", no_init) // no_init required because of const member
        .def_readwrite("max_speed", &Rocket::max_speed)
        .def_readwrite("price", &Rocket::price)
        // .def_readonly("number_of_engines", &Rocket::number_of_engines)
        .def_readwrite("number_of_engines", &Rocket::number_of_engines)
        .def_readwrite("name", &Rocket::name);

    def("make_rocket_v1", make_rocket_v1);

    def("launch_rocket_v1", launch_rocket_v1);


    class_<std::vector<Rocket>>("vector_Rocket")
        .def(vector_indexing_suite<std::vector<Rocket>>());
    def("vector_to_python", vector_to_python);

    def("vector_star_to_python", vector_star_to_python, return_value_policy<reference_existing_object>()); // Note: reference_existing_object works for both pointers and references


    class_<std::map<std::string,Rocket>>("map_stringRocket")
        .def(map_indexing_suite<std::map<std::string,Rocket>>());
    def("map_to_python", map_to_python);

}
