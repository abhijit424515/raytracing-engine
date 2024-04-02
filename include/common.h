#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

// Usings

using namespace std;

// Constants

const double infinity = numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility

inline double degrees_to_rads(double degrees) { return degrees * pi / 180.0; }

inline double random_double() {
  // Returns a random real in [0,1).
  return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
  // Returns a random real in [min,max).
  return min + (max - min) * random_double();
}

#endif