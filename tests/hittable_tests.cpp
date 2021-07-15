#include "hittable.h"
#include "aarect.h"
#include "sphere.h"

// TODO put in dependency on gtest?
#define EXPECT_NEAR(a, b, tol) assert(abs(a - b) < tol);

void test_translate_importance_sampling()
{
  // Debugging Translate's importance sampling methods
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

int main()
{
  test_translate_importance_sampling();

  return 0;
}