#include <iostream>

struct Rocket {
    double       max_speed;
    long         price;
    unsigned int number_of_engines;
    char const*  name;
};

inline Rocket construct_rocket(char const* name, unsigned int number_of_engines) {
    return {
        .max_speed = 1000*number_of_engines,
        .price = 100000000*number_of_engines,
        .number_of_engines = number_of_engines,
        .name = name 
    };
}

inline void launch_rocket(Rocket r, char const* when, char const* where) {
    std::cout << "Launching " << r.name << ". Location = " << when << ". Time = " << where << ".\n";
}
