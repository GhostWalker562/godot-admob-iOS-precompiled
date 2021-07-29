/*************************************************************************/
/*  spatial.h                                                            */
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

#ifndef SPATIAL_H
#define SPATIAL_H

#include "scene/main/node.h"
#include "scene/main/scene_tree.h"

class SpatialGizmo : public Reference {

	GDCLASS(SpatialGizmo, Reference);

public:
	virtual void create() = 0;
	virtual void transform() = 0;
	virtual void clear() = 0;
	virtual void redraw() = 0;
	virtual void free() = 0;

	SpatialGizmo();
	virtual ~SpatialGizmo() {}
};

class Spatial : public Node {

	GDCLASS(Spatial, Node);
	OBJ_CATEGORY("3D");

public:
	enum SpatialFlags {
		// this is cached, and only currently kept up to date in visual instances
		// this is set if a visual instance is
		// (a) in the tree AND (b) visible via is_visible_in_tree() call
		SPATIAL_FLAG_VI_VISIBLE = 1 << 0,
	};

private:
	enum TransformDirty {
		DIRTY_NONE = 0,
		DIRTY_VECTORS = 1,
		DIRTY_LOCAL = 2,
		DIRTY_GLOBAL = 4
	};

	mutable SelfList<Node> xform_change;

	struct Data {

		// defined in Spatial::SpatialFlags
		uint32_t spatial_flags;

		mutable Transform global_transform;
		mutable Transform local_transform;
		mutable Vector3 rotation;
		mutable Vector3 scale;

		mutable int dirty;

		Viewport *viewport;

		bool toplevel_active;
		bool toplevel;
		bool inside_world;

		int children_lock;
		Spatial *parent;
		List<Spatial *> children;
		List<Spatial *>::Element *C;

		bool ignore_notification;
		bool notify_local_transform;
		bool notify_transform;

		bool visible;
		bool disable_scale;

#ifdef TOOLS_ENABLED
		Ref<SpatialGizmo> gizmo;
		bool gizmo_disabled;
		bool gizmo_dirty;
#endif

	} data;

	void _update_gizmo();
	void _notify_dirty();
	void _propagate_transform_changed(Spatial *p_origin);

	void _propagate_visibility_changed();

protected:
	_FORCE_INLINE_ void set_ignore_transform_notification(bool p_ignore) { data.ignore_notification = p_ignore; }

	_FORCE_INLINE_ void _update_local_transform() const;

	uint32_t _get_spatial_flags() const { return data.spatial_flags; }
	void _replace_spatial_flags(uint32_t p_flags) { data.spatial_flags = p_flags; }
	void _set_spatial_flag(uint32_t p_flag, bool p_set) {
		if (p_set) {
			data.spatial_flags |= p_flag;
		} else {
			data.spatial_flags &= ~p_flag;
		}
	}

	void _notification(int p_what);
	static void _bind_methods();

public:
	enum {

		NOTIFICATION_TRANSFORM_CHANGED = SceneTree::NOTIFICATION_TRANSFORM_CHANGED,
		NOTIFICATION_ENTER_WORLD = 41,
		NOTIFICATION_EXIT_WORLD = 42,
		NOTIFICATION_VISIBILITY_CHANGED = 43,
		NOTIFICATION_LOCAL_TRANSFORM_CHANGED = 44,
	};

	Spatial *get_parent_spatial() const;

	Ref<World> get_world() const;

	void set_translation(const Vector3 &p_translation);
	void set_rotation(const Vector3 &p_euler_rad);
	void set_rotation_degrees(const Vector3 &p_euler_deg);
	void set_scale(const Vector3 &p_scale);

	Vector3 get_translation() const;
	Vector3 get_rotation() const;
	Vector3 get_rotation_degrees() const;
	Vector3 get_scale() const;

	void set_transform(const Transform &p_transform);
	void set_global_transform(const Transform &p_transform);

	Transform get_transform() const;
	Transform get_global_transform() const;

#ifdef TOOLS_ENABLED
	virtual Transform get_global_gizmo_transform() const;
	virtual Transform get_local_gizmo_transform() const;
#endif

	void set_as_toplevel(bool p_enabled);
	bool is_set_as_toplevel() const;

	void set_disable_scale(bool p_enabled);
	bool is_scale_disabled() const;

	void set_disable_gizmo(bool p_enabled);
	void update_gizmo();
	void set_gizmo(const Ref<SpatialGizmo> &p_gizmo);
	Ref<SpatialGizmo> get_gizmo() const;

	_FORCE_INLINE_ bool is_inside_world() const { return data.inside_world; }

	Transform get_relative_transform(const Node *p_parent) const;

	void rotate(const Vector3 &p_axis, float p_angle);
	void rotate_x(float p_angle);
	void rotate_y(float p_angle);
	void rotate_z(float p_angle);
	void translate(const Vector3 &p_offset);
	void scale(const Vector3 &p_ratio);

	void rotate_object_local(const Vector3 &p_axis, float p_angle);
	void scale_object_local(const Vector3 &p_scale);
	void translate_object_local(const Vector3 &p_offset);

	void global_rotate(const Vector3 &p_axis, float p_angle);
	void global_scale(const Vector3 &p_scale);
	void global_translate(const Vector3 &p_offset);

	void look_at(const Vector3 &p_target, const Vector3 &p_up);
	void look_at_from_position(const Vector3 &p_pos, const Vector3 &p_target, const Vector3 &p_up);

	Vector3 to_local(Vector3 p_global) const;
	Vector3 to_global(Vector3 p_local) const;

	void set_notify_transform(bool p_enable);
	bool is_transform_notification_enabled() const;

	void set_notify_local_transform(bool p_enable);
	bool is_local_transform_notification_enabled() const;

	void orthonormalize();
	void set_identity();

	void set_visible(bool p_visible);
	bool is_visible() const;
	void show();
	void hide();
	bool is_visible_in_tree() const;

	void force_update_transform();

	Spatial();
	~Spatial();
};

#endif
