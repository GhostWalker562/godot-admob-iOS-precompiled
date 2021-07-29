/*************************************************************************/
/*  navigation_mesh_editor_plugin.h                                      */
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

#ifndef NAVIGATION_MESH_GENERATOR_PLUGIN_H
#define NAVIGATION_MESH_GENERATOR_PLUGIN_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "navigation_mesh_generator.h"

class NavigationMeshEditor : public Control {
	friend class NavigationMeshEditorPlugin;

	GDCLASS(NavigationMeshEditor, Control);

	AcceptDialog *err_dialog;

	HBoxContainer *bake_hbox;
	ToolButton *button_bake;
	ToolButton *button_reset;
	Label *bake_info;

	NavigationMeshInstance *node;

	void _bake_pressed();
	void _clear_pressed();

protected:
	void _node_removed(Node *p_node);
	static void _bind_methods();
	void _notification(int p_option);

public:
	void edit(NavigationMeshInstance *p_nav_mesh_instance);
	NavigationMeshEditor();
	~NavigationMeshEditor();
};

class NavigationMeshEditorPlugin : public EditorPlugin {

	GDCLASS(NavigationMeshEditorPlugin, EditorPlugin);

	NavigationMeshEditor *navigation_mesh_editor;
	EditorNode *editor;

public:
	virtual String get_name() const { return "NavigationMesh"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	NavigationMeshEditorPlugin(EditorNode *p_node);
	~NavigationMeshEditorPlugin();
};

#endif // NAVIGATION_MESH_GENERATOR_PLUGIN_H
