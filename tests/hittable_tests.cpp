#include "hittable.h"
#include "sphere.h"

// TODO put in dependency on gtest?

void test_translate_importance_sampling()
{
  // Debugging Translate's importance sampling methods
  auto orig_sphere = make_shared<Sphere>(Point3(2, 5, 10), /*radius*/ 0.1, nullptr);
  auto moved_sphere = make_shared<Translate>(orig_sphere, Vec3(0, +20, 0));
  auto ref_sphere = make_shared<Sphere>(Point3(2, 25, 10), /*radius*/ 0.1, nullptr);

  auto origin = Point3(0, 0, 0);

  const auto orig_vec = orig_sphere->random(origin);
  const auto moved_vec = moved_sphere->random(origin);
  const auto ref_vec = ref_sphere->random(origin);

  std::cout << "original sphere random: " << orig_vec * (orig_sphere->center(0) - origin).length() << ", " << orig_sphere->pdf_value(origin, orig_vec) << std::endl;
  std::cout << "moved sphere random: " << moved_vec * (ref_sphere->center(0) - origin).length() << ", " << moved_sphere->pdf_value(origin, moved_vec) << std::endl;
  std::cout << "ref sphere random: " << ref_vec * (ref_sphere->center(0) - origin).length() << ", " << ref_sphere->pdf_value(origin, ref_vec) << std::endl;
}

int main()
{
  test_translate_importance_sampling();
  return 0;
}