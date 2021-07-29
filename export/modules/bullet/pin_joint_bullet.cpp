/*************************************************************************/
/*  pin_joint_bullet.cpp                                                 */
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

#include "pin_joint_bullet.h"

#include "bullet_types_converter.h"
#include "rigid_body_bullet.h"

#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>

/**
	@author AndreaCatania
*/

PinJointBullet::PinJointBullet(RigidBodyBullet *p_body_a, const Vector3 &p_pos_a, RigidBodyBullet *p_body_b, const Vector3 &p_pos_b) :
		JointBullet() {
	if (p_body_b) {

		btVector3 btPivotA;
		btVector3 btPivotB;
		G_TO_B(p_pos_a * p_body_a->get_body_scale(), btPivotA);
		G_TO_B(p_pos_b * p_body_b->get_body_scale(), btPivotB);
		p2pConstraint = bulletnew(btPoint2PointConstraint(*p_body_a->get_bt_rigid_body(),
				*p_body_b->get_bt_rigid_body(),
				btPivotA,
				btPivotB));
	} else {
		btVector3 btPivotA;
		G_TO_B(p_pos_a, btPivotA);
		p2pConstraint = bulletnew(btPoint2PointConstraint(*p_body_a->get_bt_rigid_body(), btPivotA));
	}

	setup(p2pConstraint);
}

PinJointBullet::~PinJointBullet() {}

void PinJointBullet::set_param(PhysicsServer::PinJointParam p_param, real_t p_value) {
	switch (p_param) {
		case PhysicsServer::PIN_JOINT_BIAS:
			p2pConstraint->m_setting.m_tau = p_value;
			break;
		case PhysicsServer::PIN_JOINT_DAMPING:
			p2pConstraint->m_setting.m_damping = p_value;
			break;
		case PhysicsServer::PIN_JOINT_IMPULSE_CLAMP:
			p2pConstraint->m_setting.m_impulseClamp = p_value;
			break;
	}
}

real_t PinJointBullet::get_param(PhysicsServer::PinJointParam p_param) const {
	switch (p_param) {
		case PhysicsServer::PIN_JOINT_BIAS:
			return p2pConstraint->m_setting.m_tau;
		case PhysicsServer::PIN_JOINT_DAMPING:
			return p2pConstraint->m_setting.m_damping;
		case PhysicsServer::PIN_JOINT_IMPULSE_CLAMP:
			return p2pConstraint->m_setting.m_impulseClamp;
		default:
			WARN_DEPRECATED_MSG("The parameter " + itos(p_param) + " is deprecated.");
			return 0;
	}
}

void PinJointBullet::setPivotInA(const Vector3 &p_pos) {
	btVector3 btVec;
	G_TO_B(p_pos, btVec);
	p2pConstraint->setPivotA(btVec);
}

void PinJointBullet::setPivotInB(const Vector3 &p_pos) {
	btVector3 btVec;
	G_TO_B(p_pos, btVec);
	p2pConstraint->setPivotB(btVec);
}

Vector3 PinJointBullet::getPivotInA() {
	btVector3 vec = p2pConstraint->getPivotInA();
	Vector3 gVec;
	B_TO_G(vec, gVec);
	return gVec;
}

Vector3 PinJointBullet::getPivotInB() {
	btVector3 vec = p2pConstraint->getPivotInB();
	Vector3 gVec;
	B_TO_G(vec, gVec);
	return gVec;
}
