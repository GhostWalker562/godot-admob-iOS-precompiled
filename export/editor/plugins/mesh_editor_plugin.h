/*************************************************************************/
/*  mesh_editor_plugin.h                                                 */
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

#ifndef MESH_EDITOR_PLUGIN_H
#define MESH_EDITOR_PLUGIN_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/3d/camera.h"
#include "scene/3d/light.h"
#include "scene/3d/mesh_instance.h"
#include "scene/gui/viewport_container.h"
#include "scene/resources/material.h"

class MeshEditor : public ViewportContainer {

	GDCLASS(MeshEditor, ViewportContainer);

	float rot_x;
	float rot_y;

	Viewport *viewport;
	MeshInstance *mesh_instance;
	Spatial *rotation;
	DirectionalLight *light1;
	DirectionalLight *light2;
	Camera *camera;

	Ref<Mesh> mesh;

	TextureButton *light_1_switch;
	TextureButton *light_2_switch;

	void _button_pressed(Node *p_button);
	bool first_enter;

	void _update_rotation();

protected:
	void _notification(int p_what);
	void _gui_input(Ref<InputEvent> p_event);
	static void _bind_methods();

public:
	void edit(Ref<Mesh> p_mesh);
	MeshEditor();
};

class EditorInspectorPluginMesh : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginMesh, EditorInspectorPlugin);

public:
	virtual bool can_handle(Object *p_object);
	virtual void parse_begin(Object *p_object);
};

class MeshEditorPlugin : public EditorPlugin {

	GDCLASS(MeshEditorPlugin, EditorPlugin);

public:
	virtual String get_name() const { return "Mesh"; }

	MeshEditorPlugin(EditorNode *p_node);
};

#endif
