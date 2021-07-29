/*************************************************************************/
/*  bvh_abb.h                                                            */
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

#ifndef BVH_ABB_H
#define BVH_ABB_H

// special optimized version of axis aligned bounding box
struct BVH_ABB {
	struct ConvexHull {
		// convex hulls (optional)
		const Plane *planes;
		int num_planes;
		const Vector3 *points;
		int num_points;
	};

	struct Segment {
		Vector3 from;
		Vector3 to;
	};

	enum IntersectResult {
		IR_MISS = 0,
		IR_PARTIAL,
		IR_FULL,
	};

	// we store mins with a negative value in order to test them with SIMD
	Vector3 min;
	Vector3 neg_max;

	bool operator==(const BVH_ABB &o) const { return (min == o.min) && (neg_max == o.neg_max); }
	bool operator!=(const BVH_ABB &o) const { return (*this == o) == false; }

	void set(const Vector3 &_min, const Vector3 &_max) {
		min = _min;
		neg_max = -_max;
	}

	// to and from standard AABB
	void from(const AABB &p_aabb) {
		min = p_aabb.position;
		neg_max = -(p_aabb.position + p_aabb.size);
	}

	void to(AABB &r_aabb) const {
		r_aabb.position = min;
		r_aabb.size = calculate_size();
	}

	void merge(const BVH_ABB &p_o) {
		neg_max.x = MIN(neg_max.x, p_o.neg_max.x);
		neg_max.y = MIN(neg_max.y, p_o.neg_max.y);
		neg_max.z = MIN(neg_max.z, p_o.neg_max.z);

		min.x = MIN(min.x, p_o.min.x);
		min.y = MIN(min.y, p_o.min.y);
		min.z = MIN(min.z, p_o.min.z);
	}

	Vector3 calculate_size() const {
		return -neg_max - min;
	}

	Vector3 calculate_centre() const {
		return Vector3((calculate_size() * 0.5) + min);
	}

	real_t get_proximity_to(const BVH_ABB &p_b) const {
		const Vector3 d = (min - neg_max) - (p_b.min - p_b.neg_max);
		return (Math::abs(d.x) + Math::abs(d.y) + Math::abs(d.z));
	}

	int select_by_proximity(const BVH_ABB &p_a, const BVH_ABB &p_b) const {
		return (get_proximity_to(p_a) < get_proximity_to(p_b) ? 0 : 1);
	}

	uint32_t find_cutting_planes(const BVH_ABB::ConvexHull &p_hull, uint32_t *p_plane_ids) const {
		uint32_t count = 0;

		for (int n = 0; n < p_hull.num_planes; n++) {
			const Plane &p = p_hull.planes[n];
			if (intersects_plane(p)) {
				p_plane_ids[count++] = n;
			}
		}

		return count;
	}

	bool intersects_plane(const Plane &p_p) const {
		Vector3 size = calculate_size();
		Vector3 half_extents = size * 0.5;
		Vector3 ofs = min + half_extents;

		// forward side of plane?
		Vector3 point_offset(
				(p_p.normal.x < 0) ? -half_extents.x : half_extents.x,
				(p_p.normal.y < 0) ? -half_extents.y : half_extents.y,
				(p_p.normal.z < 0) ? -half_extents.z : half_extents.z);
		Vector3 point = point_offset + ofs;

		if (!p_p.is_point_over(point))
			return false;

		point = -point_offset + ofs;
		if (p_p.is_point_over(point))
			return false;

		return true;
	}

	bool intersects_convex_optimized(const ConvexHull &p_hull, const uint32_t *p_plane_ids, uint32_t p_num_planes) const {
		Vector3 size = calculate_size();
		Vector3 half_extents = size * 0.5;
		Vector3 ofs = min + half_extents;

		for (unsigned int i = 0; i < p_num_planes; i++) {

			const Plane &p = p_hull.planes[p_plane_ids[i]];
			Vector3 point(
					(p.normal.x > 0) ? -half_extents.x : half_extents.x,
					(p.normal.y > 0) ? -half_extents.y : half_extents.y,
					(p.normal.z > 0) ? -half_extents.z : half_extents.z);
			point += ofs;
			if (p.is_point_over(point))
				return false;
		}

		return true;
	}

	bool intersects_convex_partial(const ConvexHull &p_hull) const {
		AABB bb;
		to(bb);
		return bb.intersects_convex_shape(p_hull.planes, p_hull.num_planes, p_hull.points, p_hull.num_points);
	}

	IntersectResult intersects_convex(const ConvexHull &p_hull) const {
		if (intersects_convex_partial(p_hull)) {
			// fully within? very important for tree checks
			if (is_within_convex(p_hull)) {
				return IR_FULL;
			}

			return IR_PARTIAL;
		}

		return IR_MISS;
	}

	bool is_within_convex(const ConvexHull &p_hull) const {
		// use half extents routine
		AABB bb;
		to(bb);
		return bb.inside_convex_shape(p_hull.planes, p_hull.num_planes);
	}

	bool is_point_within_hull(const ConvexHull &p_hull, const Vector3 &p_pt) const {
		for (int n = 0; n < p_hull.num_planes; n++) {
			if (p_hull.planes[n].distance_to(p_pt) > 0.0f)
				return false;
		}
		return true;
	}

	bool intersects_segment(const Segment &p_s) const {
		AABB bb;
		to(bb);
		return bb.intersects_segment(p_s.from, p_s.to);
	}

	bool intersects_point(const Vector3 &p_pt) const {
		if (_vector3_any_lessthan(-p_pt, neg_max)) return false;
		if (_vector3_any_lessthan(p_pt, min)) return false;
		return true;
	}

	bool intersects(const BVH_ABB &p_o) const {
		if (_vector3_any_morethan(p_o.min, -neg_max)) return false;
		if (_vector3_any_morethan(min, -p_o.neg_max)) return false;
		return true;
	}

	bool is_other_within(const BVH_ABB &p_o) const {
		if (_vector3_any_lessthan(p_o.neg_max, neg_max)) return false;
		if (_vector3_any_lessthan(p_o.min, min)) return false;
		return true;
	}

	void grow(const Vector3 &p_change) {
		neg_max -= p_change;
		min -= p_change;
	}

	void expand(real_t p_change) {
		grow(Vector3(p_change, p_change, p_change));
	}

	float get_area() const // actually surface area metric
	{
		Vector3 d = calculate_size();
		return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
	}
	void set_to_max_opposite_extents() {
		neg_max = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		min = neg_max;
	}

	bool _vector3_any_morethan(const Vector3 &p_a, const Vector3 &p_b) const {
		if (p_a.x > p_b.x) return true;
		if (p_a.y > p_b.y) return true;
		if (p_a.z > p_b.z) return true;
		return false;
	}

	bool _vector3_any_lessthan(const Vector3 &p_a, const Vector3 &p_b) const {
		if (p_a.x < p_b.x) return true;
		if (p_a.y < p_b.y) return true;
		if (p_a.z < p_b.z) return true;
		return false;
	}
};

#endif // BVH_ABB_H
