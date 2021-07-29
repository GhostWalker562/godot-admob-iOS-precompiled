/*************************************************************************/
/*  canvas_layer.h                                                       */
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

#ifndef CANVAS_LAYER_H
#define CANVAS_LAYER_H

#include "scene/main/node.h"
#include "scene/resources/world_2d.h"

class Viewport;
class CanvasLayer : public Node {

	GDCLASS(CanvasLayer, Node);

	bool locrotscale_dirty;
	Vector2 ofs;
	Size2 scale;
	real_t rot;
	int layer;
	Transform2D transform;
	RID canvas;

	ObjectID custom_viewport_id; // to check validity
	Viewport *custom_viewport;

	RID viewport;
	Viewport *vp;

	int sort_index;

	bool follow_viewport;
	float follow_viewport_scale;

	void _update_xform();
	void _update_locrotscale();
	void _update_follow_viewport(bool p_force_exit = false);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_layer(int p_xform);
	int get_layer() const;

	void set_transform(const Transform2D &p_xform);
	Transform2D get_transform() const;

	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;

	void set_rotation(real_t p_radians);
	real_t get_rotation() const;

	void set_rotation_degrees(real_t p_degrees);
	real_t get_rotation_degrees() const;

	void set_scale(const Size2 &p_scale);
	Size2 get_scale() const;

	Size2 get_viewport_size() const;

	RID get_viewport() const;

	void set_custom_viewport(Node *p_viewport);
	Node *get_custom_viewport() const;

	void reset_sort_index();
	int get_sort_index();

	void set_follow_viewport(bool p_enable);
	bool is_following_viewport() const;

	void set_follow_viewport_scale(float p_ratio);
	float get_follow_viewport_scale() const;

	RID get_canvas() const;

	CanvasLayer();
	~CanvasLayer();
};

#endif // CANVAS_LAYER_H
