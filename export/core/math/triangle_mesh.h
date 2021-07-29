/*************************************************************************/
/*  triangle_mesh.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

#include "core/math/face3.h"
#include "core/reference.h"

class TriangleMesh : public Reference {

	GDCLASS(TriangleMesh, Reference);

	struct Triangle {

		Vector3 normal;
		int indices[3];
	};

	PoolVector<Triangle> triangles;
	PoolVector<Vector3> vertices;

	struct BVH {

		AABB aabb;
		Vector3 center; //used for sorting
		int left;
		int right;

		int face_index;
	};

	struct BVHCmpX {

		bool operator()(const BVH *p_left, const BVH *p_right) const {

			return p_left->center.x < p_right->center.x;
		}
	};

	struct BVHCmpY {

		bool operator()(const BVH *p_left, const BVH *p_right) const {

			return p_left->center.y < p_right->center.y;
		}
	};
	struct BVHCmpZ {

		bool operator()(const BVH *p_left, const BVH *p_right) const {

			return p_left->center.z < p_right->center.z;
		}
	};

	int _create_bvh(BVH *p_bvh, BVH **p_bb, int p_from, int p_size, int p_depth, int &max_depth, int &max_alloc);

	PoolVector<BVH> bvh;
	int max_depth;
	bool valid;

public:
	bool is_valid() const;
	bool intersect_segment(const Vector3 &p_begin, const Vector3 &p_end, Vector3 &r_point, Vector3 &r_normal) const;
	bool intersect_ray(const Vector3 &p_begin, const Vector3 &p_dir, Vector3 &r_point, Vector3 &r_normal) const;
	bool intersect_convex_shape(const Plane *p_planes, int p_plane_count, const Vector3 *p_points, int p_point_count) const;
	bool inside_convex_shape(const Plane *p_planes, int p_plane_count, const Vector3 *p_points, int p_point_count, Vector3 p_scale = Vector3(1, 1, 1)) const;
	Vector3 get_area_normal(const AABB &p_aabb) const;
	PoolVector<Face3> get_faces() const;

	PoolVector<Triangle> get_triangles() const { return triangles; }
	PoolVector<Vector3> get_vertices() const { return vertices; }
	void get_indices(PoolVector<int> *r_triangles_indices) const;

	void create(const PoolVector<Vector3> &p_faces);
	TriangleMesh();
};

#endif // TRIANGLE_MESH_H
