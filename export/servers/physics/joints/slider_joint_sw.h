/*************************************************************************/
/*  slider_joint_sw.h                                                    */
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

/*
Adapted to Godot from the Bullet library.
*/

#ifndef SLIDER_JOINT_SW_H
#define SLIDER_JOINT_SW_H

#include "servers/physics/joints/jacobian_entry_sw.h"
#include "servers/physics/joints_sw.h"

/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
Added by Roman Ponomarev (rponom@gmail.com)
April 04, 2008

*/

#define SLIDER_CONSTRAINT_DEF_SOFTNESS (real_t(1.0))
#define SLIDER_CONSTRAINT_DEF_DAMPING (real_t(1.0))
#define SLIDER_CONSTRAINT_DEF_RESTITUTION (real_t(0.7))

//-----------------------------------------------------------------------------

class SliderJointSW : public JointSW {
protected:
	union {
		struct {
			BodySW *A;
			BodySW *B;
		};

		BodySW *_arr[2];
	};

	Transform m_frameInA;
	Transform m_frameInB;

	// linear limits
	real_t m_lowerLinLimit;
	real_t m_upperLinLimit;
	// angular limits
	real_t m_lowerAngLimit;
	real_t m_upperAngLimit;
	// softness, restitution and damping for different cases
	// DirLin - moving inside linear limits
	// LimLin - hitting linear limit
	// DirAng - moving inside angular limits
	// LimAng - hitting angular limit
	// OrthoLin, OrthoAng - against constraint axis
	real_t m_softnessDirLin;
	real_t m_restitutionDirLin;
	real_t m_dampingDirLin;
	real_t m_softnessDirAng;
	real_t m_restitutionDirAng;
	real_t m_dampingDirAng;
	real_t m_softnessLimLin;
	real_t m_restitutionLimLin;
	real_t m_dampingLimLin;
	real_t m_softnessLimAng;
	real_t m_restitutionLimAng;
	real_t m_dampingLimAng;
	real_t m_softnessOrthoLin;
	real_t m_restitutionOrthoLin;
	real_t m_dampingOrthoLin;
	real_t m_softnessOrthoAng;
	real_t m_restitutionOrthoAng;
	real_t m_dampingOrthoAng;

	// for interlal use
	bool m_solveLinLim;
	bool m_solveAngLim;

	JacobianEntrySW m_jacLin[3];
	real_t m_jacLinDiagABInv[3];

	JacobianEntrySW m_jacAng[3];

	real_t m_timeStep;
	Transform m_calculatedTransformA;
	Transform m_calculatedTransformB;

	Vector3 m_sliderAxis;
	Vector3 m_realPivotAInW;
	Vector3 m_realPivotBInW;
	Vector3 m_projPivotInW;
	Vector3 m_delta;
	Vector3 m_depth;
	Vector3 m_relPosA;
	Vector3 m_relPosB;

	real_t m_linPos;

	real_t m_angDepth;
	real_t m_kAngle;

	bool m_poweredLinMotor;
	real_t m_targetLinMotorVelocity;
	real_t m_maxLinMotorForce;
	real_t m_accumulatedLinMotorImpulse;

	bool m_poweredAngMotor;
	real_t m_targetAngMotorVelocity;
	real_t m_maxAngMotorForce;
	real_t m_accumulatedAngMotorImpulse;

	//------------------------
	void initParams();

public:
	// constructors
	SliderJointSW(BodySW *rbA, BodySW *rbB, const Transform &frameInA, const Transform &frameInB);
	//SliderJointSW();
	// overrides

	// access
	const BodySW *getRigidBodyA() const { return A; }
	const BodySW *getRigidBodyB() const { return B; }
	const Transform &getCalculatedTransformA() const { return m_calculatedTransformA; }
	const Transform &getCalculatedTransformB() const { return m_calculatedTransformB; }
	const Transform &getFrameOffsetA() const { return m_frameInA; }
	const Transform &getFrameOffsetB() const { return m_frameInB; }
	Transform &getFrameOffsetA() { return m_frameInA; }
	Transform &getFrameOffsetB() { return m_frameInB; }
	real_t getLowerLinLimit() { return m_lowerLinLimit; }
	void setLowerLinLimit(real_t lowerLimit) { m_lowerLinLimit = lowerLimit; }
	real_t getUpperLinLimit() { return m_upperLinLimit; }
	void setUpperLinLimit(real_t upperLimit) { m_upperLinLimit = upperLimit; }
	real_t getLowerAngLimit() { return m_lowerAngLimit; }
	void setLowerAngLimit(real_t lowerLimit) { m_lowerAngLimit = lowerLimit; }
	real_t getUpperAngLimit() { return m_upperAngLimit; }
	void setUpperAngLimit(real_t upperLimit) { m_upperAngLimit = upperLimit; }

	real_t getSoftnessDirLin() { return m_softnessDirLin; }
	real_t getRestitutionDirLin() { return m_restitutionDirLin; }
	real_t getDampingDirLin() { return m_dampingDirLin; }
	real_t getSoftnessDirAng() { return m_softnessDirAng; }
	real_t getRestitutionDirAng() { return m_restitutionDirAng; }
	real_t getDampingDirAng() { return m_dampingDirAng; }
	real_t getSoftnessLimLin() { return m_softnessLimLin; }
	real_t getRestitutionLimLin() { return m_restitutionLimLin; }
	real_t getDampingLimLin() { return m_dampingLimLin; }
	real_t getSoftnessLimAng() { return m_softnessLimAng; }
	real_t getRestitutionLimAng() { return m_restitutionLimAng; }
	real_t getDampingLimAng() { return m_dampingLimAng; }
	real_t getSoftnessOrthoLin() { return m_softnessOrthoLin; }
	real_t getRestitutionOrthoLin() { return m_restitutionOrthoLin; }
	real_t getDampingOrthoLin() { return m_dampingOrthoLin; }
	real_t getSoftnessOrthoAng() { return m_softnessOrthoAng; }
	real_t getRestitutionOrthoAng() { return m_restitutionOrthoAng; }
	real_t getDampingOrthoAng() { return m_dampingOrthoAng; }
	void setSoftnessDirLin(real_t softnessDirLin) { m_softnessDirLin = softnessDirLin; }
	void setRestitutionDirLin(real_t restitutionDirLin) { m_restitutionDirLin = restitutionDirLin; }
	void setDampingDirLin(real_t dampingDirLin) { m_dampingDirLin = dampingDirLin; }
	void setSoftnessDirAng(real_t softnessDirAng) { m_softnessDirAng = softnessDirAng; }
	void setRestitutionDirAng(real_t restitutionDirAng) { m_restitutionDirAng = restitutionDirAng; }
	void setDampingDirAng(real_t dampingDirAng) { m_dampingDirAng = dampingDirAng; }
	void setSoftnessLimLin(real_t softnessLimLin) { m_softnessLimLin = softnessLimLin; }
	void setRestitutionLimLin(real_t restitutionLimLin) { m_restitutionLimLin = restitutionLimLin; }
	void setDampingLimLin(real_t dampingLimLin) { m_dampingLimLin = dampingLimLin; }
	void setSoftnessLimAng(real_t softnessLimAng) { m_softnessLimAng = softnessLimAng; }
	void setRestitutionLimAng(real_t restitutionLimAng) { m_restitutionLimAng = restitutionLimAng; }
	void setDampingLimAng(real_t dampingLimAng) { m_dampingLimAng = dampingLimAng; }
	void setSoftnessOrthoLin(real_t softnessOrthoLin) { m_softnessOrthoLin = softnessOrthoLin; }
	void setRestitutionOrthoLin(real_t restitutionOrthoLin) { m_restitutionOrthoLin = restitutionOrthoLin; }
	void setDampingOrthoLin(real_t dampingOrthoLin) { m_dampingOrthoLin = dampingOrthoLin; }
	void setSoftnessOrthoAng(real_t softnessOrthoAng) { m_softnessOrthoAng = softnessOrthoAng; }
	void setRestitutionOrthoAng(real_t restitutionOrthoAng) { m_restitutionOrthoAng = restitutionOrthoAng; }
	void setDampingOrthoAng(real_t dampingOrthoAng) { m_dampingOrthoAng = dampingOrthoAng; }
	void setPoweredLinMotor(bool onOff) { m_poweredLinMotor = onOff; }
	bool getPoweredLinMotor() { return m_poweredLinMotor; }
	void setTargetLinMotorVelocity(real_t targetLinMotorVelocity) { m_targetLinMotorVelocity = targetLinMotorVelocity; }
	real_t getTargetLinMotorVelocity() { return m_targetLinMotorVelocity; }
	void setMaxLinMotorForce(real_t maxLinMotorForce) { m_maxLinMotorForce = maxLinMotorForce; }
	real_t getMaxLinMotorForce() { return m_maxLinMotorForce; }
	void setPoweredAngMotor(bool onOff) { m_poweredAngMotor = onOff; }
	bool getPoweredAngMotor() { return m_poweredAngMotor; }
	void setTargetAngMotorVelocity(real_t targetAngMotorVelocity) { m_targetAngMotorVelocity = targetAngMotorVelocity; }
	real_t getTargetAngMotorVelocity() { return m_targetAngMotorVelocity; }
	void setMaxAngMotorForce(real_t maxAngMotorForce) { m_maxAngMotorForce = maxAngMotorForce; }
	real_t getMaxAngMotorForce() { return m_maxAngMotorForce; }
	real_t getLinearPos() { return m_linPos; }

	// access for ODE solver
	bool getSolveLinLimit() { return m_solveLinLim; }
	real_t getLinDepth() { return m_depth[0]; }
	bool getSolveAngLimit() { return m_solveAngLim; }
	real_t getAngDepth() { return m_angDepth; }
	// shared code used by ODE solver
	void calculateTransforms(void);
	void testLinLimits(void);
	void testAngLimits(void);
	// access for PE Solver
	Vector3 getAncorInA(void);
	Vector3 getAncorInB(void);

	void set_param(PhysicsServer::SliderJointParam p_param, real_t p_value);
	real_t get_param(PhysicsServer::SliderJointParam p_param) const;

	bool setup(real_t p_step);
	void solve(real_t p_step);

	virtual PhysicsServer::JointType get_type() const { return PhysicsServer::JOINT_SLIDER; }
};

#endif // SLIDER_JOINT_SW_H
