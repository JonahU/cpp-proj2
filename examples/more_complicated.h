#include <string>
#include <vector>

using namespace std;

struct Rocket {
    double  max_speed;
    long    price;
    int     number_of_engines;
    string  name;
};

void launch(Rocket r, string when, string where);

vector<Rocket> make_rockets(int how_many) {
    return vector<Rocket>(how_many);
}
