#pragma once

#include "bvh.h"
#include "common.h"
#include "hittable.h"
#include "hittable_list.h"
#include "triangle.h"
#include "timing.h"

#include "external/tinyobjloader.h"

// TODO implement Hittable class that:
// * supports translation/rotation/scaling of mesh
// * wraps BVH, provides importance sampling
// * doesn't duplicate vertices (see PBRT)

shared_ptr<BVHNode> import_triangle_mesh(const std::string &mesh_file, shared_ptr<Material> mat_ptr)
{
  timing::Timer timer("import_triangle_mesh");

  tinyobj::ObjReader reader;
  if (!reader.ParseFromFile(mesh_file, tinyobj::ObjReaderConfig()))
  {
    if (!reader.Error().empty())
      std::cerr << "TinyObjReader: " << reader.Error();
    exit(1);
  }

  if (!reader.Warning().empty())
    std::cerr << "TinyObjReader: " << reader.Warning();

  // Loop over shapes
  const auto &shapes = reader.GetShapes();
  const auto &attrib = reader.GetAttrib();

  HittableList triangles;
  Triangle::Vertices tri_verts;

  for (size_t s = 0; s < shapes.size(); s++)
  {
    size_t num_faces = shapes[s].mesh.num_face_vertices.size();
    std::cerr << "num faces: " << num_faces << std::endl;

    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < num_faces; f++)
    {
      size_t num_vertices = size_t(shapes[s].mesh.num_face_vertices[f]);
      assert(num_vertices == 3); // we only support triangles

      // Loop over vertices in the face.
      for (size_t v = 0; v < num_vertices; v++)
      {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        tri_verts[v] = Point3(vx, vy, vz);
      }

      index_offset += num_vertices;
      triangles.add(make_shared<Triangle>(tri_verts, mat_ptr));
    }
  }

  timing::Timer bvh_timer("import_triangle_mesh/bvh");
  return make_shared<BVHNode>(triangles, /*t0*/ 0, /*t1*/ 1);
}