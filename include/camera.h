#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "material.h"

class camera {
public:
  double epsilon = 0.001; // ignoring very close ray hits (floating-point issue)
  double aspect_ratio = 16.0 / 9.0; // Ratio of image width over height
  int width = 1920;                 // Rendered image width in pixel count
  int samples_per_pixel = 10;       // Count of random samples for each pixel
  int max_depth = 10;               // Maximum number of ray bounces into scene

  double vfov = 90;                   // Vertical view angle (field of view)
  point3 lookfrom = point3(0, 0, -1); // Point camera is looking from
  point3 lookat = point3(0, 0, 0);    // Point camera is looking at
  vec3 vup = vec3(0, 1, 0);           // Camera-relative "up" direction

  double defocus_angle = 0; // Variation angle of rays through each pixel
  double focus_dist =
      10; // Distance from camera lookfrom point to plane of perfect focus

  void render(const hittable &world, bool single_thread = false) {
    initialize();

    atomic<int> processed_lines(0);
    vector<thread> threads;
    const int num_threads =
        single_thread ? 1 : thread::hardware_concurrency() * 0.8;
    const int rows_per_thread = height / num_threads;

    for (int t = 0; t < num_threads; ++t) {
      int start_height = t * rows_per_thread;
      int end_height =
          (t == num_threads - 1) ? height : (t + 1) * rows_per_thread;
      threads.emplace_back(
          [this, &world, start_height, end_height, &processed_lines]() {
            render_portion(world, start_height, end_height, processed_lines);
          });
    }

    for (auto &thread : threads)
      thread.join();

    clog << "\rDone.                 \n";
    save_image();
  }

private:
  int height;          // Rendered image height
  point3 center;       // Camera center
  point3 pixel00_loc;  // Location of pixel 0, 0
  vec3 pixel_delta_u;  // Offset to pixel to the right
  vec3 pixel_delta_v;  // Offset to pixel below
  vec3 u, v, w;        // Camera frame basis vectors
  vec3 defocus_disk_u; // Defocus disk horizontal radius
  vec3 defocus_disk_v; // Defocus disk vertical radius

  int ***image;

  void render_portion(const hittable &world, int start_height, int end_height,
                      atomic<int> &processed_lines) {
    for (int j = start_height; j < end_height; j++) {
      for (int i = 0; i < width; i++) {
        color pixel_color(0, 0, 0);
        for (int sample = 0; sample < samples_per_pixel; sample++) {
          ray r = get_ray(i, j);
          pixel_color += ray_color(r, max_depth, world);
        }
        color c = get_final_pixel_color(pixel_color / samples_per_pixel);
        image[j][i][0] = c[0];
        image[j][i][1] = c[1];
        image[j][i][2] = c[2];
      }
      clog << "\rProgress: " << ++processed_lines << "/" << height << flush;
    }
  }

  void initialize() {
    height = static_cast<int>(width / aspect_ratio);
    height = (height >= 1) ? height : 1;

    center = lookfrom;

    // VIEWPORT
    auto theta = degrees_to_rads(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h * focus_dist;
    auto viewport_width =
        viewport_height *
        (static_cast<double>(width) / height); // (width/height) ~ aspect_ratio

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical
    // viewport edges.
    auto vp_u = viewport_width * u;   // Viewport X-axis (rightward)
    auto vp_v = viewport_height * -v; // Viewport Y-axis (downward)

    // PIXEL DELTA VECTORS
    pixel_delta_u = vp_u / width;
    pixel_delta_v = vp_v / height;

    // ORIGIN and PIXEL LOCATION
    auto viewport_origin = center - vp_u / 2 - vp_v / 2 - focus_dist * w;
    pixel00_loc = viewport_origin + pixel_delta_u / 2 + pixel_delta_v / 2;

    // Calculate the camera defocus disk basis vectors.
    auto defocus_radius = focus_dist * tan(degrees_to_rads(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;

    // initialize image matrix
    image = new int **[height];
    for (int i = 0; i < height; i++)
      image[i] = new int *[width];

    for (int i = 0; i < height; i++)
      for (int j = 0; j < width; j++)
        image[i][j] = new int[3];
  }

  void save_image() {
    cout << "P3\n" << width << " " << height << "\n255\n";
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        cout << image[j][i][0] << " " << image[j][i][1] << " " << image[j][i][2]
             << "\n";
      }
    }
  }

  color ray_color(const ray &r, int depth, const hittable &world) const {
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;
    if (world.hit(r, interval(epsilon, infinity), rec)) {
      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered))
        return attenuation * ray_color(scattered, depth - 1, world);
      return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }

  ray get_ray(int i, int j) const {
    // Get a randomly-sampled camera ray for the pixel at location i,j,
    // originating from the camera defocus disk.

    auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
    auto pixel_sample = pixel_center + pixel_sample_square();

    auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
    auto ray_direction = pixel_sample - ray_origin;

    auto ray_time = random_double();
    return ray(ray_origin, ray_direction, ray_time);
  }

  point3 defocus_disk_sample() const {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }

  vec3 pixel_sample_square() const {
    // Returns a random point in the square surrounding a pixel at the origin.
    auto px = -0.5 + random_double();
    auto py = -0.5 + random_double();
    return (px * pixel_delta_u) + (py * pixel_delta_v);
  }
};

#endif