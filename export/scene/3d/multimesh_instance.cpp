/*************************************************************************/
/*  multimesh_instance.cpp                                               */
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

#include "multimesh_instance.h"

void MultiMeshInstance::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_multimesh", "multimesh"), &MultiMeshInstance::set_multimesh);
	ClassDB::bind_method(D_METHOD("get_multimesh"), &MultiMeshInstance::get_multimesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "multimesh", PROPERTY_HINT_RESOURCE_TYPE, "MultiMesh"), "set_multimesh", "get_multimesh");
}

void MultiMeshInstance::set_multimesh(const Ref<MultiMesh> &p_multimesh) {

	multimesh = p_multimesh;
	if (multimesh.is_valid())
		set_base(multimesh->get_rid());
	else
		set_base(RID());
}

Ref<MultiMesh> MultiMeshInstance::get_multimesh() const {

	return multimesh;
}

PoolVector<Face3> MultiMeshInstance::get_faces(uint32_t p_usage_flags) const {

	return PoolVector<Face3>();
}

AABB MultiMeshInstance::get_aabb() const {

	if (multimesh.is_null())
		return AABB();
	else
		return multimesh->get_aabb();
}

MultiMeshInstance::MultiMeshInstance() {
}

MultiMeshInstance::~MultiMeshInstance() {
}
