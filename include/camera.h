#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "hittable.h"

class camera {
public:
  double epsilon = 0.001; // ignoring very close ray hits (floating-point issue)
  double aspect_ratio = 16.0 / 9.0; // Ratio of image width over height
  int width = 1920;                 // Rendered image width in pixel count
  int samples_per_pixel = 10;       // Count of random samples for each pixel
  int max_depth = 10;               // Maximum number of ray bounces into scene

  void render(const hittable &world) {
    initialize();

    cout << "P3\n" << width << " " << height << "\n255\n";
    for (int j = 0; j < height; j++) {
      clog << "\rScanlines remaining: " << (height - j) << " " << flush;
      for (int i = 0; i < width; i++) {
        color pixel_color(0, 0, 0);
        for (int sample = 0; sample < samples_per_pixel; ++sample) {
          ray r = get_ray(i, j);
          pixel_color += ray_color(r, max_depth, world);
        }
        write_color(cout, pixel_color, samples_per_pixel);
      }
    }
    clog << "\rDone.                 \n";
  }

private:
  int height;         // Rendered image height
  point3 center;      // Camera center
  point3 pixel00_loc; // Location of pixel 0, 0
  vec3 pixel_delta_u; // Offset to pixel to the right
  vec3 pixel_delta_v; // Offset to pixel below

  void initialize() {
    height = static_cast<int>(width / aspect_ratio);
    height = (height >= 1) ? height : 1;

    center = point3(0, 0, 0);

    // VIEWPORT
    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width =
        viewport_height *
        (static_cast<double>(width) / height); // (width/height) ~ aspect_ratio
    auto vp_u = vec3(viewport_width, 0, 0);    // Viewport X-axis (rightward)
    auto vp_v = vec3(0, -viewport_height, 0);  // Viewport Y-axis (downward)

    // PIXEL DELTA VECTORS
    pixel_delta_u = vp_u / width;
    pixel_delta_v = vp_v / height;

    // ORIGIN and PIXEL LOCATION
    auto viewport_origin =
        center - vp_u / 2 - vp_v / 2 - vec3(0, 0, focal_length);
    pixel00_loc = viewport_origin + pixel_delta_u / 2 + pixel_delta_v / 2;
  }

  color ray_color(const ray &r, int depth, const hittable &world) const {
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;
    if (world.hit(r, interval(epsilon, infinity), rec)) {
      vec3 direction = rec.normal + random_unit_vector();
      return 0.5 * ray_color(ray(rec.p, direction), depth - 1, world);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }

  ray get_ray(int i, int j) const {
    // Get a randomly sampled camera ray for the pixel at location i,j.

    auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
    auto pixel_sample = pixel_center + pixel_sample_square();

    auto ray_origin = center;
    auto ray_direction = pixel_sample - center;
    return ray(ray_origin, ray_direction);
  }

  vec3 pixel_sample_square() const {
    // Returns a random point in the square surrounding a pixel at the origin.
    auto px = -0.5 + random_double();
    auto py = -0.5 + random_double();
    return (px * pixel_delta_u) + (py * pixel_delta_v);
  }
};

#endif