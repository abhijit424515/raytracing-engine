#include "include/camera.h"
#include "include/hittable_list.h"
#include "include/sphere.h"

int main() {
  // WORLD
  hittable_list world;
  world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
  world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

  camera cam;
  cam.render(world);
}