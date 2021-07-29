/*************************************************************************/
/*  audio_stream_editor_plugin.cpp                                       */
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

#include "audio_stream_editor_plugin.h"

#include "core/io/resource_loader.h"
#include "core/project_settings.h"
#include "editor/audio_stream_preview.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"

void AudioStreamEditor::_notification(int p_what) {

	if (p_what == NOTIFICATION_READY) {
		AudioStreamPreviewGenerator::get_singleton()->connect("preview_updated", this, "_preview_changed");
	}

	if (p_what == NOTIFICATION_THEME_CHANGED || p_what == NOTIFICATION_ENTER_TREE) {
		_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
		_stop_button->set_icon(get_icon("Stop", "EditorIcons"));
		_preview->set_frame_color(get_color("dark_color_2", "Editor"));
		set_frame_color(get_color("dark_color_1", "Editor"));

		_indicator->update();
		_preview->update();
	}

	if (p_what == NOTIFICATION_PROCESS) {
		_current = _player->get_playback_position();
		_indicator->update();
	}

	if (p_what == NOTIFICATION_VISIBILITY_CHANGED) {
		if (!is_visible_in_tree()) {
			_stop();
		}
	}
}

void AudioStreamEditor::_draw_preview() {
	Rect2 rect = _preview->get_rect();
	Size2 size = get_size();

	Ref<AudioStreamPreview> preview = AudioStreamPreviewGenerator::get_singleton()->generate_preview(stream);
	float preview_len = preview->get_length();

	Vector<Vector2> lines;
	lines.resize(size.width * 2);

	for (int i = 0; i < size.width; i++) {

		float ofs = i * preview_len / size.width;
		float ofs_n = (i + 1) * preview_len / size.width;
		float max = preview->get_max(ofs, ofs_n) * 0.5 + 0.5;
		float min = preview->get_min(ofs, ofs_n) * 0.5 + 0.5;

		int idx = i;
		lines.write[idx * 2 + 0] = Vector2(i + 1, rect.position.y + min * rect.size.y);
		lines.write[idx * 2 + 1] = Vector2(i + 1, rect.position.y + max * rect.size.y);
	}

	Vector<Color> color;
	color.push_back(get_color("contrast_color_2", "Editor"));

	VS::get_singleton()->canvas_item_add_multiline(_preview->get_canvas_item(), lines, color);
}

void AudioStreamEditor::_preview_changed(ObjectID p_which) {

	if (stream.is_valid() && stream->get_instance_id() == p_which) {
		_preview->update();
	}
}

void AudioStreamEditor::_changed_callback(Object *p_changed, const char *p_prop) {

	if (!is_visible())
		return;
	update();
}

void AudioStreamEditor::_play() {

	if (_player->is_playing()) {
		// '_pausing' variable indicates that we want to pause the audio player, not stop it. See '_on_finished()'.
		_pausing = true;
		_player->stop();
		_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
		set_process(false);
	} else {
		_player->play(_current);
		_play_button->set_icon(get_icon("Pause", "EditorIcons"));
		set_process(true);
	}
}

void AudioStreamEditor::_stop() {

	_player->stop();
	_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
	_current = 0;
	_indicator->update();
	set_process(false);
}

void AudioStreamEditor::_on_finished() {

	_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
	if (!_pausing) {
		_current = 0;
		_indicator->update();
	} else {
		_pausing = false;
	}
	set_process(false);
}

void AudioStreamEditor::_draw_indicator() {

	if (!stream.is_valid()) {
		return;
	}

	Rect2 rect = _preview->get_rect();
	float len = stream->get_length();
	float ofs_x = _current / len * rect.size.width;
	_indicator->draw_line(Point2(ofs_x, 0), Point2(ofs_x, rect.size.height), get_color("accent_color", "Editor"), 1);

	_current_label->set_text(String::num(_current, 2).pad_decimals(2) + " /");
}

void AudioStreamEditor::_on_input_indicator(Ref<InputEvent> p_event) {
	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			_seek_to(mb->get_position().x);
		}
		_dragging = mb->is_pressed();
	}

	Ref<InputEventMouseMotion> mm = p_event;

	if (mm.is_valid()) {
		if (_dragging) {
			_seek_to(mm->get_position().x);
		}
	}
}

void AudioStreamEditor::_seek_to(real_t p_x) {
	_current = p_x / _preview->get_rect().size.x * stream->get_length();
	_current = CLAMP(_current, 0, stream->get_length());
	_player->seek(_current);
	_indicator->update();
}

void AudioStreamEditor::edit(Ref<AudioStream> p_stream) {

	if (!stream.is_null())
		stream->remove_change_receptor(this);

	stream = p_stream;
	_player->set_stream(stream);
	_current = 0;
	String text = String::num(stream->get_length(), 2).pad_decimals(2) + "s";
	_duration_label->set_text(text);

	if (!stream.is_null()) {
		stream->add_change_receptor(this);
		update();
	} else {
		hide();
	}
}

void AudioStreamEditor::_bind_methods() {

	ClassDB::bind_method(D_METHOD("_preview_changed"), &AudioStreamEditor::_preview_changed);
	ClassDB::bind_method(D_METHOD("_play"), &AudioStreamEditor::_play);
	ClassDB::bind_method(D_METHOD("_stop"), &AudioStreamEditor::_stop);
	ClassDB::bind_method(D_METHOD("_on_finished"), &AudioStreamEditor::_on_finished);
	ClassDB::bind_method(D_METHOD("_draw_preview"), &AudioStreamEditor::_draw_preview);
	ClassDB::bind_method(D_METHOD("_draw_indicator"), &AudioStreamEditor::_draw_indicator);
	ClassDB::bind_method(D_METHOD("_on_input_indicator"), &AudioStreamEditor::_on_input_indicator);
}

AudioStreamEditor::AudioStreamEditor() {

	set_custom_minimum_size(Size2(1, 100) * EDSCALE);

	_player = memnew(AudioStreamPlayer);
	_player->connect("finished", this, "_on_finished");
	add_child(_player);

	VBoxContainer *vbox = memnew(VBoxContainer);
	vbox->set_anchors_and_margins_preset(PRESET_WIDE, PRESET_MODE_MINSIZE, 0);
	add_child(vbox);

	_preview = memnew(ColorRect);
	_preview->set_v_size_flags(SIZE_EXPAND_FILL);
	_preview->connect("draw", this, "_draw_preview");
	vbox->add_child(_preview);

	_indicator = memnew(Control);
	_indicator->set_anchors_and_margins_preset(PRESET_WIDE);
	_indicator->connect("draw", this, "_draw_indicator");
	_indicator->connect("gui_input", this, "_on_input_indicator");
	_preview->add_child(_indicator);

	HBoxContainer *hbox = memnew(HBoxContainer);
	hbox->add_constant_override("separation", 0);
	vbox->add_child(hbox);

	_play_button = memnew(ToolButton);
	hbox->add_child(_play_button);
	_play_button->set_focus_mode(Control::FOCUS_NONE);
	_play_button->connect("pressed", this, "_play");

	_stop_button = memnew(ToolButton);
	hbox->add_child(_stop_button);
	_stop_button->set_focus_mode(Control::FOCUS_NONE);
	_stop_button->connect("pressed", this, "_stop");

	_current_label = memnew(Label);
	_current_label->set_align(Label::ALIGN_RIGHT);
	_current_label->set_h_size_flags(SIZE_EXPAND_FILL);
	_current_label->add_font_override("font", EditorNode::get_singleton()->get_gui_base()->get_font("status_source", "EditorFonts"));
	_current_label->set_modulate(Color(1, 1, 1, 0.5));
	hbox->add_child(_current_label);

	_duration_label = memnew(Label);
	_duration_label->add_font_override("font", EditorNode::get_singleton()->get_gui_base()->get_font("status_source", "EditorFonts"));
	hbox->add_child(_duration_label);
}

void AudioStreamEditorPlugin::edit(Object *p_object) {

	AudioStream *s = Object::cast_to<AudioStream>(p_object);
	if (!s)
		return;

	audio_editor->edit(Ref<AudioStream>(s));
}

bool AudioStreamEditorPlugin::handles(Object *p_object) const {

	return p_object->is_class("AudioStream");
}

void AudioStreamEditorPlugin::make_visible(bool p_visible) {

	audio_editor->set_visible(p_visible);
}

AudioStreamEditorPlugin::AudioStreamEditorPlugin(EditorNode *p_node) {

	editor = p_node;
	audio_editor = memnew(AudioStreamEditor);
	add_control_to_container(CONTAINER_PROPERTY_EDITOR_BOTTOM, audio_editor);
	audio_editor->hide();
}

AudioStreamEditorPlugin::~AudioStreamEditorPlugin() {
}
