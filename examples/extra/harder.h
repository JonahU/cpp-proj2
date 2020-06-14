#include <map>
#include <string>
#include <vector>

struct Rocket {
    double       max_speed;
    long         price;
    unsigned int number_of_engines;
    std::string  name;
};

struct LunarRover {
    Rocket*     my_location;
};

inline void launch(Rocket r, std::string& when, std::string& where);

inline void launch_many(std::vector<Rocket>& rs, std::string& when, std::string& where);

inline void launch_test(unsigned char *& value);

inline std::vector<Rocket>* make_rockets(int how_many);

inline std::map<std::string, Rocket>* get_rockets();

inline Rocket apollo11;

inline std::vector<const char*> my_rockets;

short func() {
    /* IMPL */
    return 0;
}

std::map<LunarRover, unsigned char const*> get_rovers(std::map<Rocket, LunarRover> const&);

std::map<Rocket const*, std::string const&> something_else() {
    /* IMPL */
    return {};
}

std::map<Rocket const*, const char*>* something_else2() {
    /* IMPL */
    return nullptr;
}

int i;
