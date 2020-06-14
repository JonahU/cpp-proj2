#include <vector>

void comment() { /* This won't work, it's a bug*/ }
const std::vector<int> getints(int i, int j);

void comment2() { /* This works ok*/ }
std::vector<int> const getints2(int i, int j);

void comment3() { /* This works ok*/ }
const int getint();

void comment4() { /* This works ok*/ }
int const getint2();
