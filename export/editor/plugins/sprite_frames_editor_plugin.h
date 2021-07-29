/*************************************************************************/
/*  sprite_frames_editor_plugin.h                                        */
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

#ifndef SPRITE_FRAMES_EDITOR_PLUGIN_H
#define SPRITE_FRAMES_EDITOR_PLUGIN_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/2d/animated_sprite.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/split_container.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/tree.h"

class SpriteFramesEditor : public HSplitContainer {

	GDCLASS(SpriteFramesEditor, HSplitContainer);

	ToolButton *load;
	ToolButton *load_sheet;
	ToolButton *_delete;
	ToolButton *copy;
	ToolButton *paste;
	ToolButton *empty;
	ToolButton *empty2;
	ToolButton *move_up;
	ToolButton *move_down;
	ItemList *tree;
	bool loading_scene;
	int sel;

	HSplitContainer *split;
	ToolButton *new_anim;
	ToolButton *remove_anim;

	Tree *animations;
	SpinBox *anim_speed;
	CheckButton *anim_loop;

	EditorFileDialog *file;

	AcceptDialog *dialog;

	SpriteFrames *frames;

	StringName edited_anim;

	ConfirmationDialog *delete_dialog;

	ConfirmationDialog *split_sheet_dialog;
	ScrollContainer *splite_sheet_scroll;
	TextureRect *split_sheet_preview;
	SpinBox *split_sheet_h;
	SpinBox *split_sheet_v;
	EditorFileDialog *file_split_sheet;
	Set<int> frames_selected;
	int last_frame_selected;

	void _load_pressed();
	void _load_scene_pressed();
	void _file_load_request(const PoolVector<String> &p_path, int p_at_pos = -1);
	void _copy_pressed();
	void _paste_pressed();
	void _empty_pressed();
	void _empty2_pressed();
	void _delete_pressed();
	void _up_pressed();
	void _down_pressed();
	void _update_library(bool p_skip_selector = false);

	void _animation_select();
	void _animation_name_edited();
	void _animation_add();
	void _animation_remove();
	void _animation_remove_confirmed();
	void _animation_loop_changed();
	void _animation_fps_changed(double p_value);

	bool updating;

	UndoRedo *undo_redo;

	bool _is_drop_valid(const Dictionary &p_drag_data, const Dictionary &p_item_data) const;
	Variant get_drag_data_fw(const Point2 &p_point, Control *p_from);
	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);

	void _open_sprite_sheet();
	void _prepare_sprite_sheet(const String &p_file);
	void _sheet_preview_draw();
	void _sheet_spin_changed(double);
	void _sheet_preview_input(const Ref<InputEvent> &p_event);
	void _sheet_add_frames();
	void _sheet_select_clear_all_frames();

protected:
	void _notification(int p_what);
	void _gui_input(Ref<InputEvent> p_event);
	static void _bind_methods();

public:
	void set_undo_redo(UndoRedo *p_undo_redo) { undo_redo = p_undo_redo; }

	void edit(SpriteFrames *p_frames);
	SpriteFramesEditor();
};

class SpriteFramesEditorPlugin : public EditorPlugin {

	GDCLASS(SpriteFramesEditorPlugin, EditorPlugin);

	SpriteFramesEditor *frames_editor;
	EditorNode *editor;
	Button *button;

public:
	virtual String get_name() const { return "SpriteFrames"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	SpriteFramesEditorPlugin(EditorNode *p_node);
	~SpriteFramesEditorPlugin();
};

#endif // SPRITE_FRAMES_EDITOR_PLUGIN_H
