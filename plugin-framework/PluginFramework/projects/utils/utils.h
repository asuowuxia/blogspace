#include "apr-1/apr.h"
#include <utility>
#include <vector>

typedef std::pair<int, int> Position; 

double distance(Position p1, Position p2);

Position findClosest(Position p1, Position p2, int movement);
