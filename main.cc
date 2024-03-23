#include <bits/stdc++.h>
#include "headers/vec3.h"
using namespace std;

void write_color(const color &c) {
  cout << static_cast<int>(255.999 * c.x()) << ' '
       << static_cast<int>(255.999 * c.y()) << ' '
       << static_cast<int>(255.999 * c.z()) << '\n';
}

int main() {
  int width = 1024, height = 1024;

  cout << "P3\n" << width << " " << height << "\n255\n";

  for (int i = 0; i < height; i++) {
    clog << "\rScanlines remaining: " << (height - i) << " " << flush;
    for (int j = 0; j < width; j++) {
      color c = color(double(i) / (width - 1), double(j) / (height - 1), 0);
      write_color(c);
    }
  }

  clog << "\rDone.                 \n";
}