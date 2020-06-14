#include <iostream>

struct Rocket {
    double       max_speed;
    long         price;
    unsigned int number_of_engines;
    char const*  name;
};

Rocket construct_rocket(char const* name, unsigned int number_of_engines);

inline void launch_rocket(Rocket r, char const* when, char const* where) {
    std::cout << "Launching " << r.name << ". Location = " << when << ". Time = " << where << ".\n";
}
