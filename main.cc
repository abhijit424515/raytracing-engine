#include "include/color.h"
#include "include/ray.h"
#include "include/vec3.h"
#include <bits/stdc++.h>
using namespace std;

// color ray_color(const ray &r) { return color(0, 0, 0); }
color ray_color(const ray &r) {
  vec3 unit_direction = unit_vector(r.direction());
  auto a = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main() {
  // IMAGE
  auto aspect_ratio = 16.0 / 9.0;
  auto width = 1920;
  auto height = static_cast<int>(width / aspect_ratio);
  height = (height >= 1) ? height : 1;

  // CAMERA
  auto focal_length = 1.0;
  auto camera_center = point3(0, 0, 0);

  // VIEWPORT
  auto viewport_height = 2.0;
  auto viewport_width =
      viewport_height *
      (static_cast<double>(width) / height); // (width/height) ~ aspect_ratio
  auto vp_u = vec3(viewport_width, 0, 0);    // Viewport X-axis (rightward)
  auto vp_v = vec3(0, -viewport_height, 0);  // Viewport Y-axis (downward)

  // PIXEL DELTA VECTORS
  auto pixel_delta_u = vp_u / width;
  auto pixel_delta_v = vp_v / height;

  // ORIGIN and PIXEL LOCATION
  auto viewport_origin =
      camera_center - vp_u / 2 - vp_v / 2 - vec3(0, 0, focal_length);
  auto pixel00_loc = viewport_origin + pixel_delta_u / 2 + pixel_delta_v / 2;

  // RENDER
  cout << "P3\n" << width << " " << height << "\n255\n";
  for (int j = 0; j < height; j++) {
    clog << "\rScanlines remaining: " << (height - j) << " " << flush;
    for (int i = 0; i < width; i++) {
      auto pixel_center = pixel00_loc + i * pixel_delta_u + j * pixel_delta_v;
      auto ray_dir = pixel_center - camera_center;
      ray r(camera_center, ray_dir);

      color pixel_color = ray_color(r);
      write_color(cout, pixel_color);
    }
  }
  clog << "\rDone.                 \n";
}