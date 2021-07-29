/*************************************************************************/
/*  animation_bezier_editor.cpp                                          */
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

#include "animation_bezier_editor.h"

#include "editor/editor_node.h"
#include "editor_scale.h"

float AnimationBezierTrackEdit::_bezier_h_to_pixel(float p_h) {
	float h = p_h;
	h = (h - v_scroll) / v_zoom;
	h = (get_size().height / 2) - h;
	return h;
}

static _FORCE_INLINE_ Vector2 _bezier_interp(real_t t, const Vector2 &start, const Vector2 &control_1, const Vector2 &control_2, const Vector2 &end) {
	/* Formula from Wikipedia article on Bezier curves. */
	real_t omt = (1.0 - t);
	real_t omt2 = omt * omt;
	real_t omt3 = omt2 * omt;
	real_t t2 = t * t;
	real_t t3 = t2 * t;

	return start * omt3 + control_1 * omt2 * t * 3.0 + control_2 * omt * t2 * 3.0 + end * t3;
}

void AnimationBezierTrackEdit::_draw_track(int p_track, const Color &p_color) {

	float scale = timeline->get_zoom_scale();
	int limit = timeline->get_name_limit();
	int right_limit = get_size().width - timeline->get_buttons_width();

	//selection may have altered the order of keys
	Map<float, int> key_order;

	for (int i = 0; i < animation->track_get_key_count(p_track); i++) {
		float ofs = animation->track_get_key_time(p_track, i);
		if (moving_selection && track == p_track && selection.has(i)) {
			ofs += moving_selection_offset.x;
		}

		key_order[ofs] = i;
	}

	for (Map<float, int>::Element *E = key_order.front(); E; E = E->next()) {

		int i = E->get();

		if (!E->next())
			break;

		int i_n = E->next()->get();

		float offset = animation->track_get_key_time(p_track, i);
		float height = animation->bezier_track_get_key_value(p_track, i);
		Vector2 out_handle = animation->bezier_track_get_key_out_handle(p_track, i);
		if (track == p_track && moving_handle != 0 && moving_handle_key == i) {
			out_handle = moving_handle_right;
		}

		if (moving_selection && track == p_track && selection.has(i)) {
			offset += moving_selection_offset.x;
			height += moving_selection_offset.y;
		}

		out_handle += Vector2(offset, height);

		float offset_n = animation->track_get_key_time(p_track, i_n);
		float height_n = animation->bezier_track_get_key_value(p_track, i_n);
		Vector2 in_handle = animation->bezier_track_get_key_in_handle(p_track, i_n);
		if (track == p_track && moving_handle != 0 && moving_handle_key == i_n) {
			in_handle = moving_handle_left;
		}

		if (moving_selection && track == p_track && selection.has(i_n)) {
			offset_n += moving_selection_offset.x;
			height_n += moving_selection_offset.y;
		}

		in_handle += Vector2(offset_n, height_n);

		Vector2 start(offset, height);
		Vector2 end(offset_n, height_n);

		int from_x = (offset - timeline->get_value()) * scale + limit;
		int point_start = from_x;
		int to_x = (offset_n - timeline->get_value()) * scale + limit;
		int point_end = to_x;

		if (from_x > right_limit) //not visible
			continue;

		if (to_x < limit) //not visible
			continue;

		from_x = MAX(from_x, limit);
		to_x = MIN(to_x, right_limit);

		Vector<Vector2> lines;

		Vector2 prev_pos;

		for (int j = from_x; j <= to_x; j++) {

			float t = (j - limit) / scale + timeline->get_value();

			float h;

			if (j == point_end) {
				h = end.y; //make sure it always connects
			} else if (j == point_start) {
				h = start.y; //make sure it always connects
			} else { //custom interpolation, used because it needs to show paths affected by moving the selection or handles
				int iterations = 10;
				float low = 0;
				float high = 1;
				float middle;

				//narrow high and low as much as possible
				for (int k = 0; k < iterations; k++) {

					middle = (low + high) / 2;

					Vector2 interp = _bezier_interp(middle, start, out_handle, in_handle, end);

					if (interp.x < t) {
						low = middle;
					} else {
						high = middle;
					}
				}

				//interpolate the result:
				Vector2 low_pos = _bezier_interp(low, start, out_handle, in_handle, end);
				Vector2 high_pos = _bezier_interp(high, start, out_handle, in_handle, end);

				float c = (t - low_pos.x) / (high_pos.x - low_pos.x);

				h = low_pos.linear_interpolate(high_pos, c).y;
			}

			h = _bezier_h_to_pixel(h);

			Vector2 pos(j, h);

			if (j > from_x) {
				lines.push_back(prev_pos);
				lines.push_back(pos);
			}
			prev_pos = pos;
		}

		if (lines.size() >= 2) {
			draw_multiline(lines, p_color);
		}
	}
}

void AnimationBezierTrackEdit::_draw_line_clipped(const Vector2 &p_from, const Vector2 &p_to, const Color &p_color, int p_clip_left, int p_clip_right) {

	Vector2 from = p_from;
	Vector2 to = p_to;

	if (from.x == to.x)
		return;
	if (to.x < from.x) {
		SWAP(to, from);
	}

	if (to.x < p_clip_left)
		return;

	if (from.x > p_clip_right)
		return;

	if (to.x > p_clip_right) {
		float c = (p_clip_right - from.x) / (to.x - from.x);
		to = from.linear_interpolate(to, c);
	}

	if (from.x < p_clip_left) {
		float c = (p_clip_left - from.x) / (to.x - from.x);
		from = from.linear_interpolate(to, c);
	}

	draw_line(from, to, p_color);
}

void AnimationBezierTrackEdit::_notification(int p_what) {

	if (p_what == NOTIFICATION_THEME_CHANGED || p_what == NOTIFICATION_ENTER_TREE) {
		bezier_icon = get_icon("KeyBezierPoint", "EditorIcons");
		bezier_handle_icon = get_icon("KeyBezierHandle", "EditorIcons");
		selected_icon = get_icon("KeyBezierSelected", "EditorIcons");
		if (handle_mode_option->get_item_count() == 0) {
			handle_mode_option->add_icon_item(get_icon("BezierHandlesFree", "EditorIcons"), TTR("Free"), HANDLE_MODE_FREE);
			handle_mode_option->add_icon_item(get_icon("BezierHandlesBalanced", "EditorIcons"), TTR("Balanced"), HANDLE_MODE_BALANCED);
			handle_mode_option->add_icon_item(get_icon("BezierHandlesMirror", "EditorIcons"), TTR("Mirror"), HANDLE_MODE_MIRROR);
		}
	}
	if (p_what == NOTIFICATION_RESIZED) {

		int right_limit = get_size().width - timeline->get_buttons_width();
		int hsep = get_constant("hseparation", "ItemList");
		int vsep = get_constant("vseparation", "ItemList");

		handle_mode_option->set_position(Vector2(right_limit + hsep, get_size().height - handle_mode_option->get_combined_minimum_size().height - vsep));
		handle_mode_option->set_size(Vector2(timeline->get_buttons_width() - hsep * 2, handle_mode_option->get_combined_minimum_size().height));
	}
	if (p_what == NOTIFICATION_DRAW) {
		if (animation.is_null())
			return;

		int limit = timeline->get_name_limit();

		if (has_focus()) {
			Color accent = get_color("accent_color", "Editor");
			accent.a *= 0.7;
			draw_rect(Rect2(Point2(), get_size()), accent, false);
		}

		Ref<Font> font = get_font("font", "Label");
		Color color = get_color("font_color", "Label");
		int hsep = get_constant("hseparation", "ItemList");
		int vsep = get_constant("vseparation", "ItemList");
		Color linecolor = color;
		linecolor.a = 0.2;

		draw_line(Point2(limit, 0), Point2(limit, get_size().height), linecolor);

		int right_limit = get_size().width - timeline->get_buttons_width();

		draw_line(Point2(right_limit, 0), Point2(right_limit, get_size().height), linecolor);

		Ref<Texture> close_icon = get_icon("Close", "EditorIcons");

		close_icon_rect.position = Vector2(get_size().width - close_icon->get_width() - hsep, hsep);
		close_icon_rect.size = close_icon->get_size();
		draw_texture(close_icon, close_icon_rect.position);

		String base_path = animation->track_get_path(track);
		int end = base_path.find(":");
		if (end != -1) {
			base_path = base_path.substr(0, end + 1);
		}

		// NAMES AND ICON
		int vofs = vsep;
		int margin = 0;

		{
			NodePath path = animation->track_get_path(track);

			Node *node = NULL;

			if (root && root->has_node(path)) {
				node = root->get_node(path);
			}

			String text;

			int h = font->get_height();

			if (node) {
				int ofs = 0;

				Ref<Texture> icon = EditorNode::get_singleton()->get_object_icon(node, "Node");

				h = MAX(h, icon->get_height());

				draw_texture(icon, Point2(ofs, vofs + int(h - icon->get_height()) / 2));

				margin = icon->get_width();

				text = node->get_name();
				ofs += hsep;
				ofs += icon->get_width();

				Vector2 string_pos = Point2(ofs, vofs + (h - font->get_height()) / 2 + font->get_ascent());
				string_pos = string_pos.floor();
				draw_string(font, string_pos, text, color, limit - ofs - hsep);

				vofs += h + vsep;
			}
		}

		// RELATED TRACKS TITLES

		Map<int, Color> subtrack_colors;
		subtracks.clear();

		for (int i = 0; i < animation->get_track_count(); i++) {
			if (animation->track_get_type(i) != Animation::TYPE_BEZIER)
				continue;
			String path = animation->track_get_path(i);
			if (!path.begins_with(base_path))
				continue; //another node
			path = path.replace_first(base_path, "");

			Color cc = color;
			Rect2 rect = Rect2(margin, vofs, limit - margin - hsep, font->get_height() + vsep);
			if (i != track) {
				cc.a *= 0.7;
				uint32_t hash = path.hash();
				hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
				hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
				hash = (hash >> 16) ^ hash;
				float h = (hash % 65535) / 65536.0;
				Color subcolor;
				subcolor.set_hsv(h, 0.2, 0.8);
				subcolor.a = 0.5;
				draw_rect(Rect2(0, vofs + font->get_height() * 0.1, margin - hsep, font->get_height() * 0.8), subcolor);
				subtrack_colors[i] = subcolor;

				subtracks[i] = rect;
			} else {
				Color ac = get_color("accent_color", "Editor");
				ac.a = 0.5;
				draw_rect(rect, ac);
			}
			draw_string(font, Point2(margin, vofs + font->get_ascent()), path, cc, limit - margin - hsep);

			vofs += font->get_height() + vsep;
		}

		Color accent = get_color("accent_color", "Editor");

		{ //guides
			float min_left_scale = font->get_height() + vsep;

			float scale = (min_left_scale * 2) * v_zoom;
			float step = Math::pow(10.0, Math::round(Math::log(scale / 5.0) / Math::log(10.0))) * 5.0;
			scale = Math::stepify(scale, step);

			while (scale / v_zoom < min_left_scale * 2) {
				scale += step;
			}

			bool first = true;
			int prev_iv = 0;
			for (int i = font->get_height(); i < get_size().height; i++) {

				float ofs = get_size().height / 2 - i;
				ofs *= v_zoom;
				ofs += v_scroll;

				int iv = int(ofs / scale);
				if (ofs < 0)
					iv -= 1;
				if (!first && iv != prev_iv) {

					Color lc = linecolor;
					lc.a *= 0.5;
					draw_line(Point2(limit, i), Point2(right_limit, i), lc);
					Color c = color;
					c.a *= 0.5;
					draw_string(font, Point2(limit + 8, i - 2), rtos(Math::stepify((iv + 1) * scale, step)), c);
				}

				first = false;
				prev_iv = iv;
			}
		}

		{ //draw OTHER curves

			float scale = timeline->get_zoom_scale();
			Ref<Texture> point = get_icon("KeyValue", "EditorIcons");
			for (Map<int, Color>::Element *E = subtrack_colors.front(); E; E = E->next()) {

				_draw_track(E->key(), E->get());

				for (int i = 0; i < animation->track_get_key_count(E->key()); i++) {

					float offset = animation->track_get_key_time(E->key(), i);
					float value = animation->bezier_track_get_key_value(E->key(), i);

					Vector2 pos((offset - timeline->get_value()) * scale + limit, _bezier_h_to_pixel(value));

					if (pos.x >= limit && pos.x <= right_limit) {
						draw_texture(point, pos - point->get_size() / 2, E->get());
					}
				}
			}

			//draw edited curve
			const Color highlight = get_color("highlight_color", "Editor");
			_draw_track(track, highlight);
		}

		//draw editor handles
		{

			float scale = timeline->get_zoom_scale();
			edit_points.clear();

			for (int i = 0; i < animation->track_get_key_count(track); i++) {

				float offset = animation->track_get_key_time(track, i);
				float value = animation->bezier_track_get_key_value(track, i);

				if (moving_selection && selection.has(i)) {
					offset += moving_selection_offset.x;
					value += moving_selection_offset.y;
				}

				Vector2 pos((offset - timeline->get_value()) * scale + limit, _bezier_h_to_pixel(value));

				Vector2 in_vec = animation->bezier_track_get_key_in_handle(track, i);
				if (moving_handle != 0 && moving_handle_key == i) {
					in_vec = moving_handle_left;
				}
				Vector2 pos_in = Vector2(((offset + in_vec.x) - timeline->get_value()) * scale + limit, _bezier_h_to_pixel(value + in_vec.y));

				Vector2 out_vec = animation->bezier_track_get_key_out_handle(track, i);

				if (moving_handle != 0 && moving_handle_key == i) {
					out_vec = moving_handle_right;
				}

				Vector2 pos_out = Vector2(((offset + out_vec.x) - timeline->get_value()) * scale + limit, _bezier_h_to_pixel(value + out_vec.y));

				_draw_line_clipped(pos, pos_in, accent, limit, right_limit);
				_draw_line_clipped(pos, pos_out, accent, limit, right_limit);

				EditPoint ep;
				if (pos.x >= limit && pos.x <= right_limit) {
					ep.point_rect.position = (pos - bezier_icon->get_size() / 2).floor();
					ep.point_rect.size = bezier_icon->get_size();
					if (selection.has(i)) {
						draw_texture(selected_icon, ep.point_rect.position);
						draw_string(font, ep.point_rect.position + Vector2(8, -font->get_height() - 4), TTR("Time:") + " " + rtos(Math::stepify(offset, 0.001)), accent);
						draw_string(font, ep.point_rect.position + Vector2(8, -8), TTR("Value:") + " " + rtos(Math::stepify(value, 0.001)), accent);
					} else {
						draw_texture(bezier_icon, ep.point_rect.position);
					}
					ep.point_rect = ep.point_rect.grow(ep.point_rect.size.width * 0.5);
				}
				if (pos_in.x >= limit && pos_in.x <= right_limit) {
					ep.in_rect.position = (pos_in - bezier_handle_icon->get_size() / 2).floor();
					ep.in_rect.size = bezier_handle_icon->get_size();
					draw_texture(bezier_handle_icon, ep.in_rect.position);
					ep.in_rect = ep.in_rect.grow(ep.in_rect.size.width * 0.5);
				}
				if (pos_out.x >= limit && pos_out.x <= right_limit) {
					ep.out_rect.position = (pos_out - bezier_handle_icon->get_size() / 2).floor();
					ep.out_rect.size = bezier_handle_icon->get_size();
					draw_texture(bezier_handle_icon, ep.out_rect.position);
					ep.out_rect = ep.out_rect.grow(ep.out_rect.size.width * 0.5);
				}
				edit_points.push_back(ep);
			}
		}

		if (box_selecting) {
			Color bs = accent;
			bs.a *= 0.5;
			Vector2 bs_from = box_selection_from;
			Vector2 bs_to = box_selection_to;
			if (bs_from.x > bs_to.x) {
				SWAP(bs_from.x, bs_to.x);
			}
			if (bs_from.y > bs_to.y) {
				SWAP(bs_from.y, bs_to.y);
			}
			draw_rect(Rect2(bs_from, bs_to - bs_from), bs);
		}
	}
}

Ref<Animation> AnimationBezierTrackEdit::get_animation() const {
	return animation;
}

void AnimationBezierTrackEdit::set_animation_and_track(const Ref<Animation> &p_animation, int p_track) {

	animation = p_animation;
	track = p_track;
	if (is_connected("select_key", editor, "_key_selected"))
		disconnect("select_key", editor, "_key_selected");
	if (is_connected("deselect_key", editor, "_key_deselected"))
		disconnect("deselect_key", editor, "_key_deselected");
	connect("select_key", editor, "_key_selected", varray(p_track), CONNECT_DEFERRED);
	connect("deselect_key", editor, "_key_deselected", varray(p_track), CONNECT_DEFERRED);
	update();
}

Size2 AnimationBezierTrackEdit::get_minimum_size() const {

	return Vector2(1, 1);
}

void AnimationBezierTrackEdit::set_undo_redo(UndoRedo *p_undo_redo) {
	undo_redo = p_undo_redo;
}

void AnimationBezierTrackEdit::set_timeline(AnimationTimelineEdit *p_timeline) {
	timeline = p_timeline;
	timeline->connect("zoom_changed", this, "_zoom_changed");
}
void AnimationBezierTrackEdit::set_editor(AnimationTrackEditor *p_editor) {
	editor = p_editor;
	connect("clear_selection", editor, "_clear_selection", varray(false));
}

void AnimationBezierTrackEdit::_play_position_draw() {

	if (!animation.is_valid() || play_position_pos < 0)
		return;

	float scale = timeline->get_zoom_scale();
	int h = get_size().height;

	int px = (-timeline->get_value() + play_position_pos) * scale + timeline->get_name_limit();

	if (px >= timeline->get_name_limit() && px < (get_size().width - timeline->get_buttons_width())) {
		Color color = get_color("accent_color", "Editor");
		play_position->draw_line(Point2(px, 0), Point2(px, h), color, Math::round(2 * EDSCALE));
	}
}

void AnimationBezierTrackEdit::set_play_position(float p_pos) {

	play_position_pos = p_pos;
	play_position->update();
}

void AnimationBezierTrackEdit::update_play_position() {
	play_position->update();
}

void AnimationBezierTrackEdit::set_root(Node *p_root) {
	root = p_root;
}
void AnimationBezierTrackEdit::_zoom_changed() {
	update();
	play_position->update();
}

String AnimationBezierTrackEdit::get_tooltip(const Point2 &p_pos) const {

	return Control::get_tooltip(p_pos);
}

void AnimationBezierTrackEdit::_clear_selection() {
	selection.clear();
	emit_signal("clear_selection");
	update();
}

void AnimationBezierTrackEdit::_clear_selection_for_anim(const Ref<Animation> &p_anim) {

	if (!(animation == p_anim))
		return;
	//selection.clear();
	_clear_selection();
}

void AnimationBezierTrackEdit::_select_at_anim(const Ref<Animation> &p_anim, int p_track, float p_pos) {

	if (!(animation == p_anim))
		return;

	int idx = animation->track_find_key(p_track, p_pos, true);
	ERR_FAIL_COND(idx < 0);

	selection.insert(idx);
	emit_signal("select_key", idx, true);
	update();
}

void AnimationBezierTrackEdit::_gui_input(const Ref<InputEvent> &p_event) {

	if (p_event->is_pressed()) {
		if (ED_GET_SHORTCUT("animation_editor/duplicate_selection")->is_shortcut(p_event)) {
			duplicate_selection();
			accept_event();
		}

		if (ED_GET_SHORTCUT("animation_editor/delete_selection")->is_shortcut(p_event)) {
			delete_selection();
			accept_event();
		}
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == BUTTON_WHEEL_DOWN) {
		float v_zoom_orig = v_zoom;
		if (mb->get_command()) {
			timeline->get_zoom()->set_value(timeline->get_zoom()->get_value() * 1.05);
		} else {
			if (v_zoom < 100000) {
				v_zoom *= 1.2;
			}
		}
		v_scroll = v_scroll + (mb->get_position().y - get_size().y / 2) * (v_zoom - v_zoom_orig);
		update();
	}

	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == BUTTON_WHEEL_UP) {
		float v_zoom_orig = v_zoom;
		if (mb->get_command()) {
			timeline->get_zoom()->set_value(timeline->get_zoom()->get_value() / 1.05);
		} else {
			if (v_zoom > 0.000001) {
				v_zoom /= 1.2;
			}
		}
		v_scroll = v_scroll + (mb->get_position().y - get_size().y / 2) * (v_zoom - v_zoom_orig);
		update();
	}

	if (mb.is_valid() && mb->get_button_index() == BUTTON_MIDDLE) {

		if (mb->is_pressed()) {
			int x = mb->get_position().x - timeline->get_name_limit();
			panning_timeline_from = x / timeline->get_zoom_scale();
			panning_timeline = true;
			panning_timeline_at = timeline->get_value();
		} else {
			panning_timeline = false;
		}
	}

	if (mb.is_valid() && mb->get_button_index() == BUTTON_RIGHT && mb->is_pressed()) {

		menu_insert_key = mb->get_position();
		if (menu_insert_key.x >= timeline->get_name_limit() && menu_insert_key.x <= get_size().width - timeline->get_buttons_width()) {
			Vector2 popup_pos = get_global_transform().xform(mb->get_position());

			menu->clear();
			menu->add_icon_item(bezier_icon, TTR("Insert Key Here"), MENU_KEY_INSERT);
			if (selection.size()) {
				menu->add_separator();
				menu->add_icon_item(get_icon("Duplicate", "EditorIcons"), TTR("Duplicate Selected Key(s)"), MENU_KEY_DUPLICATE);
				menu->add_separator();
				menu->add_icon_item(get_icon("Remove", "EditorIcons"), TTR("Delete Selected Key(s)"), MENU_KEY_DELETE);
			}

			menu->set_as_minsize();
			menu->set_position(popup_pos);
			menu->popup();
		}
	}

	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == BUTTON_LEFT) {

		if (close_icon_rect.has_point(mb->get_position())) {
			emit_signal("close_request");
			return;
		}
		for (Map<int, Rect2>::Element *E = subtracks.front(); E; E = E->next()) {
			if (E->get().has_point(mb->get_position())) {
				set_animation_and_track(animation, E->key());
				_clear_selection();
				return;
			}
		}

		for (int i = 0; i < edit_points.size(); i++) {

			//first check point
			//command makes it ignore the main point, so control point editors can be force-edited
			//path 2D editing in the 3D and 2D editors works the same way
			if (!mb->get_command()) {
				if (edit_points[i].point_rect.has_point(mb->get_position())) {
					if (mb->get_shift()) {
						//add to selection
						if (selection.has(i)) {
							selection.erase(i);
						} else {
							selection.insert(i);
						}
						update();
						select_single_attempt = -1;
					} else if (selection.has(i)) {
						moving_selection_attempt = true;
						moving_selection = false;
						moving_selection_from_key = i;
						moving_selection_offset = Vector2();
						select_single_attempt = i;
						update();
					} else {

						moving_selection_attempt = true;
						moving_selection = true;
						moving_selection_from_key = i;
						moving_selection_offset = Vector2();
						selection.clear();
						selection.insert(i);
						update();
					}
					return;
				}
			}

			if (edit_points[i].in_rect.has_point(mb->get_position())) {
				moving_handle = -1;
				moving_handle_key = i;
				moving_handle_left = animation->bezier_track_get_key_in_handle(track, i);
				moving_handle_right = animation->bezier_track_get_key_out_handle(track, i);
				update();
				return;
			}

			if (edit_points[i].out_rect.has_point(mb->get_position())) {
				moving_handle = 1;
				moving_handle_key = i;
				moving_handle_left = animation->bezier_track_get_key_in_handle(track, i);
				moving_handle_right = animation->bezier_track_get_key_out_handle(track, i);
				update();
				return;
				;
			}
		}

		//insert new point
		if (mb->get_command() && mb->get_position().x >= timeline->get_name_limit() && mb->get_position().x < get_size().width - timeline->get_buttons_width()) {

			Array new_point;
			new_point.resize(5);

			float h = (get_size().height / 2 - mb->get_position().y) * v_zoom + v_scroll;

			new_point[0] = h;
			new_point[1] = -0.25;
			new_point[2] = 0;
			new_point[3] = 0.25;
			new_point[4] = 0;

			float time = ((mb->get_position().x - timeline->get_name_limit()) / timeline->get_zoom_scale()) + timeline->get_value();
			while (animation->track_find_key(track, time, true) != -1) {
				time += 0.001;
			}

			undo_redo->create_action(TTR("Add Bezier Point"));
			undo_redo->add_do_method(animation.ptr(), "track_insert_key", track, time, new_point);
			undo_redo->add_undo_method(animation.ptr(), "track_remove_key_at_position", track, time);
			undo_redo->commit_action();

			//then attempt to move
			int index = animation->track_find_key(track, time, true);
			ERR_FAIL_COND(index == -1);
			_clear_selection();
			selection.insert(index);

			moving_selection_attempt = true;
			moving_selection = false;
			moving_selection_from_key = index;
			moving_selection_offset = Vector2();
			select_single_attempt = -1;
			update();

			return;
		}

		//box select
		if (mb->get_position().x >= timeline->get_name_limit() && mb->get_position().x < get_size().width - timeline->get_buttons_width()) {
			box_selecting_attempt = true;
			box_selecting = false;
			box_selecting_add = false;
			box_selection_from = mb->get_position();
			return;
		}
	}

	if (box_selecting_attempt && mb.is_valid() && !mb->is_pressed() && mb->get_button_index() == BUTTON_LEFT) {

		if (box_selecting) {
			//do actual select
			if (!box_selecting_add) {
				_clear_selection();
			}

			Vector2 bs_from = box_selection_from;
			Vector2 bs_to = box_selection_to;
			if (bs_from.x > bs_to.x) {
				SWAP(bs_from.x, bs_to.x);
			}
			if (bs_from.y > bs_to.y) {
				SWAP(bs_from.y, bs_to.y);
			}
			Rect2 selection_rect(bs_from, bs_to - bs_from);

			for (int i = 0; i < edit_points.size(); i++) {

				if (edit_points[i].point_rect.intersects(selection_rect)) {
					selection.insert(i);
				}
			}
		} else {
			_clear_selection(); //clicked and nothing happened, so clear the selection
		}
		box_selecting_attempt = false;
		box_selecting = false;
		update();
	}

	if (moving_handle != 0 && mb.is_valid() && !mb->is_pressed() && mb->get_button_index() == BUTTON_LEFT) {

		undo_redo->create_action(TTR("Move Bezier Points"));
		undo_redo->add_do_method(animation.ptr(), "bezier_track_set_key_in_handle", track, moving_handle_key, moving_handle_left);
		undo_redo->add_do_method(animation.ptr(), "bezier_track_set_key_out_handle", track, moving_handle_key, moving_handle_right);
		undo_redo->add_undo_method(animation.ptr(), "bezier_track_set_key_in_handle", track, moving_handle_key, animation->bezier_track_get_key_in_handle(track, moving_handle_key));
		undo_redo->add_undo_method(animation.ptr(), "bezier_track_set_key_out_handle", track, moving_handle_key, animation->bezier_track_get_key_out_handle(track, moving_handle_key));
		undo_redo->commit_action();

		moving_handle = 0;
		update();
	}

	if (moving_selection_attempt && mb.is_valid() && !mb->is_pressed() && mb->get_button_index() == BUTTON_LEFT) {

		if (moving_selection) {
			//combit it

			undo_redo->create_action(TTR("Move Bezier Points"));

			List<AnimMoveRestore> to_restore;
			// 1-remove the keys
			for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

				undo_redo->add_do_method(animation.ptr(), "track_remove_key", track, E->get());
			}
			// 2- remove overlapped keys
			for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

				float newtime = editor->snap_time(animation->track_get_key_time(track, E->get()) + moving_selection_offset.x);

				int idx = animation->track_find_key(track, newtime, true);
				if (idx == -1)
					continue;

				if (selection.has(idx))
					continue; //already in selection, don't save

				undo_redo->add_do_method(animation.ptr(), "track_remove_key_at_position", track, newtime);
				AnimMoveRestore amr;

				amr.key = animation->track_get_key_value(track, idx);
				amr.track = track;
				amr.time = newtime;

				to_restore.push_back(amr);
			}

			// 3-move the keys (re insert them)
			for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

				float newpos = editor->snap_time(animation->track_get_key_time(track, E->get()) + moving_selection_offset.x);
				/*
				if (newpos<0)
					continue; //no add at the beginning
				*/
				Array key = animation->track_get_key_value(track, E->get());
				float h = key[0];
				h += moving_selection_offset.y;
				key[0] = h;
				undo_redo->add_do_method(animation.ptr(), "track_insert_key", track, newpos, key, 1);
			}

			// 4-(undo) remove inserted keys
			for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

				float newpos = editor->snap_time(animation->track_get_key_time(track, E->get()) + moving_selection_offset.x);
				/*
				if (newpos<0)
					continue; //no remove what no inserted
				*/
				undo_redo->add_undo_method(animation.ptr(), "track_remove_key_at_position", track, newpos);
			}

			// 5-(undo) reinsert keys
			for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

				float oldpos = animation->track_get_key_time(track, E->get());
				undo_redo->add_undo_method(animation.ptr(), "track_insert_key", track, oldpos, animation->track_get_key_value(track, E->get()), 1);
			}

			// 6-(undo) reinsert overlapped keys
			for (List<AnimMoveRestore>::Element *E = to_restore.front(); E; E = E->next()) {

				AnimMoveRestore &amr = E->get();
				undo_redo->add_undo_method(animation.ptr(), "track_insert_key", amr.track, amr.time, amr.key, 1);
			}

			undo_redo->add_do_method(this, "_clear_selection_for_anim", animation);
			undo_redo->add_undo_method(this, "_clear_selection_for_anim", animation);

			// 7-reselect

			for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

				float oldpos = animation->track_get_key_time(track, E->get());
				float newpos = editor->snap_time(oldpos + moving_selection_offset.x);

				undo_redo->add_do_method(this, "_select_at_anim", animation, track, newpos);
				undo_redo->add_undo_method(this, "_select_at_anim", animation, track, oldpos);
			}

			undo_redo->commit_action();

			moving_selection = false;
		} else if (select_single_attempt != -1) {
			selection.clear();
			selection.insert(select_single_attempt);
		}

		moving_selection_attempt = false;
		update();
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && mm->get_button_mask() & BUTTON_MASK_MIDDLE) {
		v_scroll += mm->get_relative().y * v_zoom;
		if (v_scroll > 100000)
			v_scroll = 100000;
		if (v_scroll < -100000)
			v_scroll = -100000;

		int x = mm->get_position().x - timeline->get_name_limit();
		float ofs = x / timeline->get_zoom_scale();
		float diff = ofs - panning_timeline_from;
		timeline->set_value(panning_timeline_at - diff);

		update();
	}
	if (moving_selection_attempt && mm.is_valid()) {

		if (!moving_selection) {
			moving_selection = true;
			select_single_attempt = -1;
		}

		float y = (get_size().height / 2 - mm->get_position().y) * v_zoom + v_scroll;
		float x = editor->snap_time(((mm->get_position().x - timeline->get_name_limit()) / timeline->get_zoom_scale()) + timeline->get_value());

		moving_selection_offset = Vector2(x - animation->track_get_key_time(track, moving_selection_from_key), y - animation->bezier_track_get_key_value(track, moving_selection_from_key));
		update();
	}

	if (box_selecting_attempt && mm.is_valid()) {

		if (!box_selecting) {
			box_selecting = true;
			box_selecting_add = mm->get_shift();
		}

		box_selection_to = mm->get_position();

		if (get_local_mouse_position().y < 0) {
			//avoid cursor from going too above, so it does not lose focus with viewport
			warp_mouse(Vector2(get_local_mouse_position().x, 0));
		}
		update();
	}

	if (moving_handle != 0 && mm.is_valid()) {

		float y = (get_size().height / 2 - mm->get_position().y) * v_zoom + v_scroll;
		float x = ((mm->get_position().x - timeline->get_name_limit()) / timeline->get_zoom_scale()) + timeline->get_value();

		Vector2 key_pos = Vector2(animation->track_get_key_time(track, moving_handle_key), animation->bezier_track_get_key_value(track, moving_handle_key));

		Vector2 moving_handle_value = Vector2(x, y) - key_pos;

		moving_handle_left = animation->bezier_track_get_key_in_handle(track, moving_handle_key);
		moving_handle_right = animation->bezier_track_get_key_out_handle(track, moving_handle_key);

		if (moving_handle == -1) {
			moving_handle_left = moving_handle_value;
			if (moving_handle_left.x > 0) {
				moving_handle_left.x = 0;
			}

			if (handle_mode_option->get_selected() == HANDLE_MODE_BALANCED) {
				Vector2 scale = Vector2(timeline->get_zoom_scale(), v_zoom);
				moving_handle_right = (-(moving_handle_left * scale).normalized() * (moving_handle_right * scale).length()) / scale;

			} else if (handle_mode_option->get_selected() == HANDLE_MODE_MIRROR) {
				moving_handle_right = -moving_handle_left;
			}
		}

		if (moving_handle == 1) {
			moving_handle_right = moving_handle_value;
			if (moving_handle_right.x < 0) {
				moving_handle_right.x = 0;
			}

			if (handle_mode_option->get_selected() == HANDLE_MODE_BALANCED) {
				Vector2 scale = Vector2(timeline->get_zoom_scale(), v_zoom);
				moving_handle_left = (-(moving_handle_right * scale).normalized() * (moving_handle_left * scale).length()) / scale;
			} else if (handle_mode_option->get_selected() == HANDLE_MODE_MIRROR) {
				moving_handle_left = -moving_handle_right;
			}
		}

		update();
	}
}

void AnimationBezierTrackEdit::_menu_selected(int p_index) {

	switch (p_index) {
		case MENU_KEY_INSERT: {

			Array new_point;
			new_point.resize(5);

			float h = (get_size().height / 2 - menu_insert_key.y) * v_zoom + v_scroll;

			new_point[0] = h;
			new_point[1] = -0.25;
			new_point[2] = 0;
			new_point[3] = 0.25;
			new_point[4] = 0;

			float time = ((menu_insert_key.x - timeline->get_name_limit()) / timeline->get_zoom_scale()) + timeline->get_value();
			while (animation->track_find_key(track, time, true) != -1) {
				time += 0.001;
			}

			undo_redo->create_action(TTR("Add Bezier Point"));
			undo_redo->add_do_method(animation.ptr(), "track_insert_key", track, time, new_point);
			undo_redo->add_undo_method(animation.ptr(), "track_remove_key_at_position", track, time);
			undo_redo->commit_action();

		} break;
		case MENU_KEY_DUPLICATE: {
			duplicate_selection();
		} break;
		case MENU_KEY_DELETE: {
			delete_selection();
		} break;
	}
}

void AnimationBezierTrackEdit::duplicate_selection() {

	if (selection.size() == 0)
		return;

	float top_time = 1e10;
	for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

		float t = animation->track_get_key_time(track, E->get());
		if (t < top_time)
			top_time = t;
	}

	undo_redo->create_action(TTR("Anim Duplicate Keys"));

	List<Pair<int, float> > new_selection_values;

	for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

		float t = animation->track_get_key_time(track, E->get());
		float dst_time = t + (timeline->get_play_position() - top_time);
		int existing_idx = animation->track_find_key(track, dst_time, true);

		undo_redo->add_do_method(animation.ptr(), "track_insert_key", track, dst_time, animation->track_get_key_value(track, E->get()), animation->track_get_key_transition(track, E->get()));
		undo_redo->add_undo_method(animation.ptr(), "track_remove_key_at_position", track, dst_time);

		Pair<int, float> p;
		p.first = track;
		p.second = dst_time;
		new_selection_values.push_back(p);

		if (existing_idx != -1) {

			undo_redo->add_undo_method(animation.ptr(), "track_insert_key", track, dst_time, animation->track_get_key_value(track, existing_idx), animation->track_get_key_transition(track, existing_idx));
		}
	}

	undo_redo->commit_action();

	//reselect duplicated

	selection.clear();
	for (List<Pair<int, float> >::Element *E = new_selection_values.front(); E; E = E->next()) {

		int track = E->get().first;
		float time = E->get().second;

		int existing_idx = animation->track_find_key(track, time, true);

		if (existing_idx == -1)
			continue;

		selection.insert(existing_idx);
	}

	update();
}

void AnimationBezierTrackEdit::delete_selection() {
	if (selection.size()) {
		undo_redo->create_action(TTR("Anim Delete Keys"));

		for (Set<int>::Element *E = selection.back(); E; E = E->prev()) {

			undo_redo->add_do_method(animation.ptr(), "track_remove_key", track, E->get());
			undo_redo->add_undo_method(animation.ptr(), "track_insert_key", track, animation->track_get_key_time(track, E->get()), animation->track_get_key_value(track, E->get()), 1);
		}
		undo_redo->add_do_method(this, "_clear_selection_for_anim", animation);
		undo_redo->add_undo_method(this, "_clear_selection_for_anim", animation);
		undo_redo->commit_action();
		//selection.clear();
	}
}

void AnimationBezierTrackEdit::set_block_animation_update_ptr(bool *p_block_ptr) {
	block_animation_update_ptr = p_block_ptr;
}

void AnimationBezierTrackEdit::_bind_methods() {

	ClassDB::bind_method("_zoom_changed", &AnimationBezierTrackEdit::_zoom_changed);
	ClassDB::bind_method("_menu_selected", &AnimationBezierTrackEdit::_menu_selected);
	ClassDB::bind_method("_gui_input", &AnimationBezierTrackEdit::_gui_input);
	ClassDB::bind_method("_play_position_draw", &AnimationBezierTrackEdit::_play_position_draw);

	ClassDB::bind_method("_clear_selection", &AnimationBezierTrackEdit::_clear_selection);
	ClassDB::bind_method("_clear_selection_for_anim", &AnimationBezierTrackEdit::_clear_selection_for_anim);
	ClassDB::bind_method("_select_at_anim", &AnimationBezierTrackEdit::_select_at_anim);

	ADD_SIGNAL(MethodInfo("timeline_changed", PropertyInfo(Variant::REAL, "position"), PropertyInfo(Variant::BOOL, "drag")));
	ADD_SIGNAL(MethodInfo("remove_request", PropertyInfo(Variant::INT, "track")));
	ADD_SIGNAL(MethodInfo("insert_key", PropertyInfo(Variant::REAL, "ofs")));
	ADD_SIGNAL(MethodInfo("select_key", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::BOOL, "single")));
	ADD_SIGNAL(MethodInfo("deselect_key", PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("clear_selection"));
	ADD_SIGNAL(MethodInfo("close_request"));

	ADD_SIGNAL(MethodInfo("move_selection_begin"));
	ADD_SIGNAL(MethodInfo("move_selection", PropertyInfo(Variant::REAL, "ofs")));
	ADD_SIGNAL(MethodInfo("move_selection_commit"));
	ADD_SIGNAL(MethodInfo("move_selection_cancel"));
}

AnimationBezierTrackEdit::AnimationBezierTrackEdit() {
	undo_redo = NULL;
	timeline = NULL;
	root = NULL;
	menu = NULL;
	block_animation_update_ptr = NULL;

	moving_selection_attempt = false;
	moving_selection = false;
	select_single_attempt = -1;
	box_selecting = false;
	box_selecting_attempt = false;

	moving_handle = 0;

	play_position_pos = 0;
	play_position = memnew(Control);
	play_position->set_mouse_filter(MOUSE_FILTER_PASS);
	add_child(play_position);
	play_position->set_anchors_and_margins_preset(PRESET_WIDE);
	play_position->connect("draw", this, "_play_position_draw");
	set_focus_mode(FOCUS_CLICK);

	v_scroll = 0;
	v_zoom = 1;

	panning_timeline = false;
	set_clip_contents(true);
	handle_mode = HANDLE_MODE_FREE;
	handle_mode_option = memnew(OptionButton);
	add_child(handle_mode_option);

	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", this, "_menu_selected");

	//set_mouse_filter(MOUSE_FILTER_PASS); //scroll has to work too for selection
}
