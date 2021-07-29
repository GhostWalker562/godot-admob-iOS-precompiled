/*************************************************************************/
/*  body_pair_2d_sw.cpp                                                  */
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

#include "body_pair_2d_sw.h"
#include "collision_solver_2d_sw.h"
#include "space_2d_sw.h"

#define POSITION_CORRECTION
#define ACCUMULATE_IMPULSES

void BodyPair2DSW::_add_contact(const Vector2 &p_point_A, const Vector2 &p_point_B, void *p_self) {

	BodyPair2DSW *self = (BodyPair2DSW *)p_self;

	self->_contact_added_callback(p_point_A, p_point_B);
}

void BodyPair2DSW::_contact_added_callback(const Vector2 &p_point_A, const Vector2 &p_point_B) {

	// check if we already have the contact

	Vector2 local_A = A->get_inv_transform().basis_xform(p_point_A);
	Vector2 local_B = B->get_inv_transform().basis_xform(p_point_B - offset_B);

	int new_index = contact_count;

	ERR_FAIL_COND(new_index >= (MAX_CONTACTS + 1));

	Contact contact;

	contact.acc_normal_impulse = 0;
	contact.acc_bias_impulse = 0;
	contact.acc_tangent_impulse = 0;
	contact.local_A = local_A;
	contact.local_B = local_B;
	contact.reused = true;
	contact.normal = (p_point_A - p_point_B).normalized();
	contact.mass_normal = 0; // will be computed in setup()

	// attempt to determine if the contact will be reused

	real_t recycle_radius_2 = space->get_contact_recycle_radius() * space->get_contact_recycle_radius();

	for (int i = 0; i < contact_count; i++) {

		Contact &c = contacts[i];
		if (
				c.local_A.distance_squared_to(local_A) < (recycle_radius_2) &&
				c.local_B.distance_squared_to(local_B) < (recycle_radius_2)) {

			contact.acc_normal_impulse = c.acc_normal_impulse;
			contact.acc_tangent_impulse = c.acc_tangent_impulse;
			contact.acc_bias_impulse = c.acc_bias_impulse;
			new_index = i;
			break;
		}
	}

	// figure out if the contact amount must be reduced to fit the new contact

	if (new_index == MAX_CONTACTS) {

		// remove the contact with the minimum depth

		int least_deep = -1;
		real_t min_depth = 1e10;

		for (int i = 0; i <= contact_count; i++) {

			Contact &c = (i == contact_count) ? contact : contacts[i];
			Vector2 global_A = A->get_transform().basis_xform(c.local_A);
			Vector2 global_B = B->get_transform().basis_xform(c.local_B) + offset_B;

			Vector2 axis = global_A - global_B;
			real_t depth = axis.dot(c.normal);

			if (depth < min_depth) {

				min_depth = depth;
				least_deep = i;
			}
		}

		ERR_FAIL_COND(least_deep == -1);

		if (least_deep < contact_count) { //replace the last deep contact by the new one

			contacts[least_deep] = contact;
		}

		return;
	}

	contacts[new_index] = contact;

	if (new_index == contact_count) {

		contact_count++;
	}
}

void BodyPair2DSW::_validate_contacts() {

	//make sure to erase contacts that are no longer valid

	real_t max_separation = space->get_contact_max_separation();
	real_t max_separation2 = max_separation * max_separation;

	for (int i = 0; i < contact_count; i++) {

		Contact &c = contacts[i];

		bool erase = false;
		if (!c.reused) {
			//was left behind in previous frame
			erase = true;
		} else {
			c.reused = false;

			Vector2 global_A = A->get_transform().basis_xform(c.local_A);
			Vector2 global_B = B->get_transform().basis_xform(c.local_B) + offset_B;
			Vector2 axis = global_A - global_B;
			real_t depth = axis.dot(c.normal);

			if (depth < -max_separation || (global_B + c.normal * depth - global_A).length_squared() > max_separation2) {
				erase = true;
			}
		}

		if (erase) {
			// contact no longer needed, remove

			if ((i + 1) < contact_count) {
				// swap with the last one
				SWAP(contacts[i], contacts[contact_count - 1]);
			}

			i--;
			contact_count--;
		}
	}
}

bool BodyPair2DSW::_test_ccd(real_t p_step, Body2DSW *p_A, int p_shape_A, const Transform2D &p_xform_A, Body2DSW *p_B, int p_shape_B, const Transform2D &p_xform_B, bool p_swap_result) {

	Vector2 motion = p_A->get_linear_velocity() * p_step;
	real_t mlen = motion.length();
	if (mlen < CMP_EPSILON)
		return false;

	Vector2 mnormal = motion / mlen;

	real_t min, max;
	p_A->get_shape(p_shape_A)->project_rangev(mnormal, p_xform_A, min, max);
	bool fast_object = mlen > (max - min) * 0.3; //going too fast in that direction

	if (!fast_object) { //did it move enough in this direction to even attempt raycast? let's say it should move more than 1/3 the size of the object in that axis
		return false;
	}

	//cast a segment from support in motion normal, in the same direction of motion by motion length
	//support is the worst case collision point, so real collision happened before
	int a;
	Vector2 s[2];
	p_A->get_shape(p_shape_A)->get_supports(p_xform_A.basis_xform(mnormal).normalized(), s, a);
	Vector2 from = p_xform_A.xform(s[0]);
	Vector2 to = from + motion;

	Transform2D from_inv = p_xform_B.affine_inverse();

	Vector2 local_from = from_inv.xform(from - mnormal * mlen * 0.1); //start from a little inside the bounding box
	Vector2 local_to = from_inv.xform(to);

	Vector2 rpos, rnorm;
	if (!p_B->get_shape(p_shape_B)->intersect_segment(local_from, local_to, rpos, rnorm))
		return false;

	//ray hit something

	Vector2 hitpos = p_xform_B.xform(rpos);

	Vector2 contact_A = to;
	Vector2 contact_B = hitpos;

	//create a contact

	if (p_swap_result)
		_contact_added_callback(contact_B, contact_A);
	else
		_contact_added_callback(contact_A, contact_B);

	return true;
}

real_t combine_bounce(Body2DSW *A, Body2DSW *B) {
	return CLAMP(A->get_bounce() + B->get_bounce(), 0, 1);
}

real_t combine_friction(Body2DSW *A, Body2DSW *B) {
	return ABS(MIN(A->get_friction(), B->get_friction()));
}

bool BodyPair2DSW::setup(real_t p_step) {

	//cannot collide
	if (!A->test_collision_mask(B) || A->has_exception(B->get_self()) || B->has_exception(A->get_self()) || (A->get_mode() <= Physics2DServer::BODY_MODE_KINEMATIC && B->get_mode() <= Physics2DServer::BODY_MODE_KINEMATIC && A->get_max_contacts_reported() == 0 && B->get_max_contacts_reported() == 0)) {
		collided = false;
		return false;
	}

	if (A->is_shape_set_as_disabled(shape_A) || B->is_shape_set_as_disabled(shape_B)) {
		collided = false;
		return false;
	}

	//use local A coordinates to avoid numerical issues on collision detection
	offset_B = B->get_transform().get_origin() - A->get_transform().get_origin();

	_validate_contacts();

	Vector2 offset_A = A->get_transform().get_origin();
	Transform2D xform_Au = A->get_transform().untranslated();
	Transform2D xform_A = xform_Au * A->get_shape_transform(shape_A);

	Transform2D xform_Bu = B->get_transform();
	xform_Bu.elements[2] -= A->get_transform().get_origin();
	Transform2D xform_B = xform_Bu * B->get_shape_transform(shape_B);

	Shape2DSW *shape_A_ptr = A->get_shape(shape_A);
	Shape2DSW *shape_B_ptr = B->get_shape(shape_B);

	Vector2 motion_A, motion_B;

	if (A->get_continuous_collision_detection_mode() == Physics2DServer::CCD_MODE_CAST_SHAPE) {
		motion_A = A->get_motion();
	}
	if (B->get_continuous_collision_detection_mode() == Physics2DServer::CCD_MODE_CAST_SHAPE) {
		motion_B = B->get_motion();
	}

	bool prev_collided = collided;

	collided = CollisionSolver2DSW::solve(shape_A_ptr, xform_A, motion_A, shape_B_ptr, xform_B, motion_B, _add_contact, this, &sep_axis);
	if (!collided) {

		//test ccd (currently just a raycast)

		if (A->get_continuous_collision_detection_mode() == Physics2DServer::CCD_MODE_CAST_RAY && A->get_mode() > Physics2DServer::BODY_MODE_KINEMATIC) {
			if (_test_ccd(p_step, A, shape_A, xform_A, B, shape_B, xform_B))
				collided = true;
		}

		if (B->get_continuous_collision_detection_mode() == Physics2DServer::CCD_MODE_CAST_RAY && B->get_mode() > Physics2DServer::BODY_MODE_KINEMATIC) {
			if (_test_ccd(p_step, B, shape_B, xform_B, A, shape_A, xform_A, true))
				collided = true;
		}

		if (!collided) {
			oneway_disabled = false;
			return false;
		}
	}

	if (oneway_disabled)
		return false;

	if (!prev_collided) {
		if (A->is_shape_set_as_one_way_collision(shape_A)) {
			Vector2 direction = xform_A.get_axis(1).normalized();
			bool valid = false;
			for (int i = 0; i < contact_count; i++) {
				Contact &c = contacts[i];
				if (!c.reused)
					continue;
				if (c.normal.dot(direction) > -CMP_EPSILON) //greater (normal inverted)
					continue;
				valid = true;
				break;
			}
			if (!valid) {
				collided = false;
				oneway_disabled = true;
				return false;
			}
		}

		if (B->is_shape_set_as_one_way_collision(shape_B)) {
			Vector2 direction = xform_B.get_axis(1).normalized();
			bool valid = false;
			for (int i = 0; i < contact_count; i++) {
				Contact &c = contacts[i];
				if (!c.reused)
					continue;
				if (c.normal.dot(direction) < CMP_EPSILON) //less (normal ok)
					continue;
				valid = true;
				break;
			}
			if (!valid) {
				collided = false;
				oneway_disabled = true;
				return false;
			}
		}
	}

	real_t max_penetration = space->get_contact_max_allowed_penetration();

	real_t bias = 0.3;
	if (shape_A_ptr->get_custom_bias() || shape_B_ptr->get_custom_bias()) {

		if (shape_A_ptr->get_custom_bias() == 0)
			bias = shape_B_ptr->get_custom_bias();
		else if (shape_B_ptr->get_custom_bias() == 0)
			bias = shape_A_ptr->get_custom_bias();
		else
			bias = (shape_B_ptr->get_custom_bias() + shape_A_ptr->get_custom_bias()) * 0.5;
	}

	cc = 0;

	real_t inv_dt = 1.0 / p_step;

	bool do_process = false;

	for (int i = 0; i < contact_count; i++) {

		Contact &c = contacts[i];

		Vector2 global_A = xform_Au.xform(c.local_A);
		Vector2 global_B = xform_Bu.xform(c.local_B);

		real_t depth = c.normal.dot(global_A - global_B);

		if (depth <= 0 || !c.reused) {
			c.active = false;
			continue;
		}

		c.active = true;
#ifdef DEBUG_ENABLED
		if (space->is_debugging_contacts()) {
			space->add_debug_contact(global_A + offset_A);
			space->add_debug_contact(global_B + offset_A);
		}
#endif
		int gather_A = A->can_report_contacts();
		int gather_B = B->can_report_contacts();

		c.rA = global_A;
		c.rB = global_B - offset_B;

		if (gather_A | gather_B) {

			//Vector2 crB( -B->get_angular_velocity() * c.rB.y, B->get_angular_velocity() * c.rB.x );

			global_A += offset_A;
			global_B += offset_A;

			if (gather_A) {
				Vector2 crB(-B->get_angular_velocity() * c.rB.y, B->get_angular_velocity() * c.rB.x);
				A->add_contact(global_A, -c.normal, depth, shape_A, global_B, shape_B, B->get_instance_id(), B->get_self(), crB + B->get_linear_velocity());
			}
			if (gather_B) {

				Vector2 crA(-A->get_angular_velocity() * c.rA.y, A->get_angular_velocity() * c.rA.x);
				B->add_contact(global_B, c.normal, depth, shape_B, global_A, shape_A, A->get_instance_id(), A->get_self(), crA + A->get_linear_velocity());
			}
		}

		if ((A->get_mode() <= Physics2DServer::BODY_MODE_KINEMATIC && B->get_mode() <= Physics2DServer::BODY_MODE_KINEMATIC)) {
			c.active = false;
			collided = false;
			continue;
		}

		// Precompute normal mass, tangent mass, and bias.
		real_t rnA = c.rA.dot(c.normal);
		real_t rnB = c.rB.dot(c.normal);
		real_t kNormal = A->get_inv_mass() + B->get_inv_mass();
		kNormal += A->get_inv_inertia() * (c.rA.dot(c.rA) - rnA * rnA) + B->get_inv_inertia() * (c.rB.dot(c.rB) - rnB * rnB);
		c.mass_normal = 1.0f / kNormal;

		Vector2 tangent = c.normal.tangent();
		real_t rtA = c.rA.dot(tangent);
		real_t rtB = c.rB.dot(tangent);
		real_t kTangent = A->get_inv_mass() + B->get_inv_mass();
		kTangent += A->get_inv_inertia() * (c.rA.dot(c.rA) - rtA * rtA) + B->get_inv_inertia() * (c.rB.dot(c.rB) - rtB * rtB);
		c.mass_tangent = 1.0f / kTangent;

		c.bias = -bias * inv_dt * MIN(0.0f, -depth + max_penetration);
		c.depth = depth;
		//c.acc_bias_impulse=0;

#ifdef ACCUMULATE_IMPULSES
		{
			// Apply normal + friction impulse
			Vector2 P = c.acc_normal_impulse * c.normal + c.acc_tangent_impulse * tangent;

			A->apply_impulse(c.rA, -P);
			B->apply_impulse(c.rB, P);
		}

#endif

		c.bounce = combine_bounce(A, B);
		if (c.bounce) {

			Vector2 crA(-A->get_angular_velocity() * c.rA.y, A->get_angular_velocity() * c.rA.x);
			Vector2 crB(-B->get_angular_velocity() * c.rB.y, B->get_angular_velocity() * c.rB.x);
			Vector2 dv = B->get_linear_velocity() + crB - A->get_linear_velocity() - crA;
			c.bounce = c.bounce * dv.dot(c.normal);
		}

		do_process = true;
	}

	return do_process;
}

void BodyPair2DSW::solve(real_t p_step) {

	if (!collided)
		return;

	for (int i = 0; i < contact_count; ++i) {

		Contact &c = contacts[i];
		cc++;

		if (!c.active)
			continue;

		// Relative velocity at contact

		Vector2 crA(-A->get_angular_velocity() * c.rA.y, A->get_angular_velocity() * c.rA.x);
		Vector2 crB(-B->get_angular_velocity() * c.rB.y, B->get_angular_velocity() * c.rB.x);
		Vector2 dv = B->get_linear_velocity() + crB - A->get_linear_velocity() - crA;

		Vector2 crbA(-A->get_biased_angular_velocity() * c.rA.y, A->get_biased_angular_velocity() * c.rA.x);
		Vector2 crbB(-B->get_biased_angular_velocity() * c.rB.y, B->get_biased_angular_velocity() * c.rB.x);
		Vector2 dbv = B->get_biased_linear_velocity() + crbB - A->get_biased_linear_velocity() - crbA;

		real_t vn = dv.dot(c.normal);
		real_t vbn = dbv.dot(c.normal);
		Vector2 tangent = c.normal.tangent();
		real_t vt = dv.dot(tangent);

		real_t jbn = (c.bias - vbn) * c.mass_normal;
		real_t jbnOld = c.acc_bias_impulse;
		c.acc_bias_impulse = MAX(jbnOld + jbn, 0.0f);

		Vector2 jb = c.normal * (c.acc_bias_impulse - jbnOld);

		A->apply_bias_impulse(c.rA, -jb);
		B->apply_bias_impulse(c.rB, jb);

		real_t jn = -(c.bounce + vn) * c.mass_normal;
		real_t jnOld = c.acc_normal_impulse;
		c.acc_normal_impulse = MAX(jnOld + jn, 0.0f);

		real_t friction = combine_friction(A, B);

		real_t jtMax = friction * c.acc_normal_impulse;
		real_t jt = -vt * c.mass_tangent;
		real_t jtOld = c.acc_tangent_impulse;
		c.acc_tangent_impulse = CLAMP(jtOld + jt, -jtMax, jtMax);

		Vector2 j = c.normal * (c.acc_normal_impulse - jnOld) + tangent * (c.acc_tangent_impulse - jtOld);

		A->apply_impulse(c.rA, -j);
		B->apply_impulse(c.rB, j);
	}
}

BodyPair2DSW::BodyPair2DSW(Body2DSW *p_A, int p_shape_A, Body2DSW *p_B, int p_shape_B) :
		Constraint2DSW(_arr, 2) {

	A = p_A;
	B = p_B;
	shape_A = p_shape_A;
	shape_B = p_shape_B;
	space = A->get_space();
	A->add_constraint(this, 0);
	B->add_constraint(this, 1);
	contact_count = 0;
	collided = false;
	oneway_disabled = false;
}

BodyPair2DSW::~BodyPair2DSW() {

	A->remove_constraint(this);
	B->remove_constraint(this);
}
