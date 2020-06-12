#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> // convert std vector -> py obj
#include <boost/python/suite/indexing/map_indexing_suite.hpp> // convert std map -> py obj

#include <iostream>
#include <string>
#include <map>
#include <vector>

struct Rocket {
    double      max_speed;
    long        price;
    int         number_of_engines;
    std::string name;
};

// conversion boilerplate
bool operator==(Rocket const& lhs, Rocket const& rhs) {
    return &lhs == &rhs;
}

void launch_rocket_v1(Rocket& r, std::string& where, char const* when) {
    std::cout
        << "launching rocket " << r.name << '\n'
        << "speed = " <<  r.max_speed << "mph\n"
        << "location = " << where << '\n'
        << "time = " << when << '\n';
}

std::vector<Rocket> vector_example() {
    std::vector<Rocket> rockets = {
        { 100.0, 333222000, 2, "Rocket v1" },
        { 200.0, 444222000, 4, "Rocket v2" },
        { 300.0, 555222000, 8, "Rocket v3" }
    };
    return rockets;
}

std::vector<Rocket>* vector_star_example() {
    std::vector<Rocket>* rockets_ptr = new std::vector<Rocket>{
        { 100.0, 333222000, 2, "Rocket v1" },
        { 200.0, 444222000, 4, "Rocket v2" },
        { 300.0, 555222000, 8, "Rocket v3" }
    };
    return rockets_ptr;
}

std::map<std::string, Rocket> map_example() {
    std::map<std::string, Rocket> rockets = {
        { "apollo11", { 100.0, 333222000, 2, "Rocket v1" } },
        { "apollo12", { 200.0, 444222000, 4, "Rocket v2" } },
        { "apollo13", { 300.0, 555222000, 8, "Rocket v3" } }
    };
    return rockets;
}

std::map<std::string, Rocket> global_map = map_example();

std::map<std::string, Rocket>& get_global_map() {
    return global_map;
}

void print_global_map() {
    std::cout << "global_map : ";
    for(auto& element: global_map) {
        std::cout << element.first << "=" << element.second.name << ", ";
    }
    std::cout << '\n';
}

BOOST_PYTHON_MODULE(container) {
    using namespace boost::python;

    class_<Rocket>("Rocket")
        .def_readwrite("max_speed", &Rocket::max_speed)
        .def_readwrite("price", &Rocket::price)
        .def_readwrite("number_of_engines", &Rocket::number_of_engines)
        .def_readwrite("name", &Rocket::name);


    def("launch_rocket_v1", launch_rocket_v1);


    class_<std::vector<Rocket>>("vector_Rocket")
        .def(vector_indexing_suite<std::vector<Rocket>>());
    def("vector_example", vector_example);

    def("vector_star_example", vector_star_example, return_value_policy<reference_existing_object>()); // Note: reference_existing_object works for both pointers and references


    class_<std::map<std::string,Rocket>>("map_stringRocket")
        .def(map_indexing_suite<std::map<std::string,Rocket>>());
    def("map_example", map_example);

    def("get_global_map", get_global_map, return_value_policy<reference_existing_object>());
    def("print_global_map", print_global_map);

}
