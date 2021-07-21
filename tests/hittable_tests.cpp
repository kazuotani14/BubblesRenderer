#include "hittable.h"
#include "aarect.h"
#include "sphere.h"
#include "triangle.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "external/tinyobjloader.h"

// TODO put in dependency on gtest?
#define EXPECT_NEAR(a, b, tol) assert(abs(a - b) < tol);
#define EXPECT_LT(a, b) assert(a < b);

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

  // std::cout << "moved_vec: " << moved_vec << std::endl;
  const double moved_pdf = moved_sphere->pdf_value(origin, moved_vec);

  // std::cout << "ref_vec: " << ref_vec << std::endl;
  const double ref_pdf = ref_sphere->pdf_value(origin, ref_vec);

  EXPECT_NEAR(ref_pdf, moved_pdf, 1e-6);
}

void test_triangle()
{
  // Basic mostly axis-aligned case
  const auto t1 = Triangle({Point3(0, 0, 1), Point3(0, 1, 1), Point3(1, 0, 1)}, nullptr);
  const auto r1 = Ray(Point3(0.2, 0.1, 0.2), Vec3(0.05, 0.1, 0.8));

  hit_record hrec;
  assert(t1.hit(r1, 0, 1, &hrec));
  // std::cout << hrec << std::endl;
  assert(hrec.front_face);

  // Hit vertex
  const auto r2 = Ray(Point3(0, 0, 0), Vec3(0, 0, 1));
  assert(t1.hit(r2, 0, 1, &hrec));

  // Hit edge
  const auto r3 = Ray(Point3(1, 0.2, 0), Vec3(-0.5, -0.2, 1));
  assert(t1.hit(r3, 0, 1, &hrec));

  // Sampling
  Point3 origin(0.5, -0.3, 2.0);
  for (int i = 0; i < 100; ++i)
  {
    const auto v_rand = t1.random(origin);
    assert(t1.hit(Ray(origin, v_rand), 0, 1, &hrec));
  }

  // Bounding box
  AABB bb;
  assert(t1.bounding_box(0, 1, &bb));
  // std::cout << bb.min() << ", " << bb.max() << std::endl;
  assert(bb.min().x() < bb.max().x());
  assert(bb.min().y() < bb.max().z());
  assert(bb.min().y() < bb.max().z());
}

void test_obj_loader()
{
  // Verifying example on their README https://github.com/tinyobjloader/tinyobjloader
  const std::string inputfile = "examples/meshes/teapot.obj";
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(inputfile, tinyobj::ObjReaderConfig()))
  {
    if (!reader.Error().empty())
    {
      std::cerr << "TinyObjReader: " << reader.Error();
    }
    exit(1);
  }

  if (!reader.Warning().empty())
  {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  // Loop over shapes
  const auto &shapes = reader.GetShapes();
  const auto &attrib = reader.GetAttrib();

  // std::cout << "num shapes: " << shapes.size() << std::endl;
  for (size_t s = 0; s < shapes.size(); s++)
  {
    size_t num_faces = shapes[s].mesh.num_face_vertices.size();
    // std::cout << "num faces: " << num_faces << std::endl;

    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < num_faces; f++)
    {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
      // std::cout << "num vertices: " << fv << std::endl;

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++)
      {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        // std::cout << vx << ", " << vy << ", " << vz << std::endl;
      }
      index_offset += fv;
    }
  }
}

int main()
{
  test_translate_importance_sampling();
  test_rotate_importance_sampling();
  test_triangle();
  test_obj_loader();
  return 0;
}