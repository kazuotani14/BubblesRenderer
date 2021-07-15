#include "hittable.h"
#include "aarect.h"
#include "sphere.h"

// TODO put in dependency on gtest?
#define EXPECT_NEAR(a, b, tol) assert(abs(a - b) < tol);

void test_translate_importance_sampling()
{
  auto orig_sphere = make_shared<Sphere>(Point3(2, 5, 10), /*radius*/ 0.1, nullptr);
  auto moved_sphere = make_shared<Translate>(orig_sphere, Vec3(0, +20, 0));
  auto ref_sphere = make_shared<Sphere>(Point3(2, 25, 10), /*radius*/ 0.1, nullptr);

  auto origin = Point3(10, 5, -20);

  const auto moved_vec = moved_sphere->random(origin);
  const auto ref_vec = ref_sphere->random(origin);

  const double moved_pdf = moved_sphere->pdf_value(origin, moved_vec);
  const double ref_pdf = ref_sphere->pdf_value(origin, ref_vec);

  EXPECT_NEAR(ref_pdf, moved_pdf, 1e-6);
}

void test_rotate_importance_sampling()
{
  auto orig_sphere = make_shared<Sphere>(Point3(1, 0, 1), /*radius*/ 0.01, nullptr);
  // auto moved_sphere = make_shared<RotateY>(orig_sphere, 90);
  // auto ref_sphere = make_shared<Sphere>(Point3(1, 0, -1), /*radius*/ 0.01, nullptr);

  auto moved_sphere = make_shared<RotateY>(orig_sphere, -45);
  auto ref_sphere = make_shared<Sphere>(Point3(0, 0, sqrt(2)), /*radius*/ 0.01, nullptr);

  auto origin = Point3(1.5, 0, 0.0);

  const auto moved_vec = moved_sphere->random(origin);
  const auto ref_vec = ref_sphere->random(origin);

  std::cout << "moved_vec: " << moved_vec << std::endl;
  const double moved_pdf = moved_sphere->pdf_value(origin, moved_vec);

  std::cout << "ref_vec: " << ref_vec << std::endl;
  const double ref_pdf = ref_sphere->pdf_value(origin, ref_vec);

  EXPECT_NEAR(ref_pdf, moved_pdf, 1e-6);
}

int main()
{
  test_translate_importance_sampling();
  test_rotate_importance_sampling();
  return 0;
}