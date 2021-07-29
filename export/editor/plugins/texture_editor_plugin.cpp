/*************************************************************************/
/*  texture_editor_plugin.cpp                                            */
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

#include "texture_editor_plugin.h"

#include "core/io/resource_loader.h"
#include "core/project_settings.h"
#include "editor/editor_settings.h"

void TextureEditor::_gui_input(Ref<InputEvent> p_event) {
}

void TextureEditor::_notification(int p_what) {

	if (p_what == NOTIFICATION_READY) {

		//get_scene()->connect("node_removed",this,"_node_removed");
	}

	if (p_what == NOTIFICATION_DRAW) {

		Ref<Texture> checkerboard = get_icon("Checkerboard", "EditorIcons");
		Size2 size = get_size();

		draw_texture_rect(checkerboard, Rect2(Point2(), size), true);

		int tex_width = texture->get_width() * size.height / texture->get_height();
		int tex_height = size.height;

		if (tex_width > size.width) {
			tex_width = size.width;
			tex_height = texture->get_height() * tex_width / texture->get_width();
		}

		// Prevent the texture from being unpreviewable after the rescale, so that we can still see something
		if (tex_height <= 0)
			tex_height = 1;
		if (tex_width <= 0)
			tex_width = 1;

		int ofs_x = (size.width - tex_width) / 2;
		int ofs_y = (size.height - tex_height) / 2;

		if (Object::cast_to<CurveTexture>(*texture)) {
			// In the case of CurveTextures we know they are 1 in height, so fill the preview to see the gradient
			ofs_y = 0;
			tex_height = size.height;
		} else if (Object::cast_to<GradientTexture>(*texture)) {
			ofs_y = size.height / 4.0;
			tex_height = size.height / 2.0;
		}

		draw_texture_rect(texture, Rect2(ofs_x, ofs_y, tex_width, tex_height));

		Ref<Font> font = get_font("font", "Label");

		String format;
		if (Object::cast_to<ImageTexture>(*texture)) {
			format = Image::get_format_name(Object::cast_to<ImageTexture>(*texture)->get_format());
		} else if (Object::cast_to<StreamTexture>(*texture)) {
			format = Image::get_format_name(Object::cast_to<StreamTexture>(*texture)->get_format());
		} else {
			format = texture->get_class();
		}
		String text = itos(texture->get_width()) + "x" + itos(texture->get_height()) + " " + format;

		Size2 rect = font->get_string_size(text);

		Vector2 draw_from = size - rect + Size2(-2, font->get_ascent() - 2);
		if (draw_from.x < 0)
			draw_from.x = 0;

		draw_string(font, draw_from + Vector2(2, 2), text, Color(0, 0, 0, 0.5), size.width);
		draw_string(font, draw_from - Vector2(2, 2), text, Color(0, 0, 0, 0.5), size.width);
		draw_string(font, draw_from, text, Color(1, 1, 1, 1), size.width);
	}
}

void TextureEditor::_changed_callback(Object *p_changed, const char *p_prop) {

	if (!is_visible())
		return;
	update();
}

void TextureEditor::edit(Ref<Texture> p_texture) {

	if (!texture.is_null())
		texture->remove_change_receptor(this);

	texture = p_texture;

	if (!texture.is_null()) {
		texture->add_change_receptor(this);
		update();
	} else {
		hide();
	}
}

void TextureEditor::_bind_methods() {

	ClassDB::bind_method(D_METHOD("_gui_input"), &TextureEditor::_gui_input);
}

TextureEditor::TextureEditor() {

	set_custom_minimum_size(Size2(1, 150));
}

TextureEditor::~TextureEditor() {
	if (!texture.is_null()) {
		texture->remove_change_receptor(this);
	}
}
//
bool EditorInspectorPluginTexture::can_handle(Object *p_object) {

	return Object::cast_to<ImageTexture>(p_object) != NULL || Object::cast_to<AtlasTexture>(p_object) != NULL || Object::cast_to<StreamTexture>(p_object) != NULL || Object::cast_to<LargeTexture>(p_object) != NULL || Object::cast_to<AnimatedTexture>(p_object) != NULL;
}

void EditorInspectorPluginTexture::parse_begin(Object *p_object) {

	Texture *texture = Object::cast_to<Texture>(p_object);
	if (!texture) {
		return;
	}
	Ref<Texture> m(texture);

	TextureEditor *editor = memnew(TextureEditor);
	editor->edit(m);
	add_custom_control(editor);
}

TextureEditorPlugin::TextureEditorPlugin(EditorNode *p_node) {

	Ref<EditorInspectorPluginTexture> plugin;
	plugin.instance();
	add_inspector_plugin(plugin);
}
