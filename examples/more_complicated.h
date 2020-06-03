#include <string>
#include <vector>
#include <map>

struct Rocket {
    double      max_speed;
    long        price;
    int         number_of_engines;
    std::string name;
};

inline void launch(Rocket r, std::string& when, std::string& where);

inline std::vector<Rocket>* make_rockets(int how_many);

inline std::map<std::string, Rocket>* get_rockets();
