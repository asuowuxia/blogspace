#include "utils.h"
#include <math.h>

double calcDistance(Position p1, Position p2)
{
  return ::sqrt((double)(p2.second - p1.second) * (p2.second - p1.second) + (p2.first - p1.first) * (p2.first - p1.first));
}

Position findClosest(Position p1, Position p2, int movement)
{
  double d = calcDistance(p1, p2);
  if (d <= movement)
    return p2;

  double ratio = movement / d;

  int dx = int((p2.first - p1.first) * ratio);
  int dy = int((p2.second - p1.second) * ratio);

  return Position(p1.first + dx, p1.second + dy);
}

