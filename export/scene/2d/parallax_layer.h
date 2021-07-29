/*************************************************************************/
/*  parallax_layer.h                                                     */
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

#ifndef PARALLAX_LAYER_H
#define PARALLAX_LAYER_H

#include "scene/2d/node_2d.h"

class ParallaxLayer : public Node2D {

	GDCLASS(ParallaxLayer, Node2D);

	Point2 orig_offset;
	Point2 orig_scale;
	Size2 motion_scale;
	Vector2 motion_offset;
	Vector2 mirroring;
	void _update_mirroring();

	Point2 screen_offset;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_motion_offset(const Size2 &p_offset);
	Size2 get_motion_offset() const;

	void set_motion_scale(const Size2 &p_scale);
	Size2 get_motion_scale() const;

	void set_mirroring(const Size2 &p_mirroring);
	Size2 get_mirroring() const;

	void set_base_offset_and_scale(const Point2 &p_offset, float p_scale, const Point2 &p_screen_offset);

	virtual String get_configuration_warning() const;
	ParallaxLayer();
};

#endif // PARALLAX_LAYER_H
