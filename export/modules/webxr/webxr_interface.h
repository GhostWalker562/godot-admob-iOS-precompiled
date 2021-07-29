/*************************************************************************/
/*  webxr_interface.h                                                    */
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

#ifndef WEBXR_INTERFACE_H
#define WEBXR_INTERFACE_H

#include "servers/arvr/arvr_interface.h"
#include "servers/arvr/arvr_positional_tracker.h"

/**
	@author David Snopek <david.snopek@snopekgames.com>

	The WebXR interface is a VR/AR interface that can be used on the web.
*/

class WebXRInterface : public ARVRInterface {
	GDCLASS(WebXRInterface, ARVRInterface);

protected:
	static void _bind_methods();

public:
	virtual void is_session_supported(const String &p_session_mode) = 0;
	virtual void set_session_mode(String p_session_mode) = 0;
	virtual String get_session_mode() const = 0;
	virtual void set_required_features(String p_required_features) = 0;
	virtual String get_required_features() const = 0;
	virtual void set_optional_features(String p_optional_features) = 0;
	virtual String get_optional_features() const = 0;
	virtual void set_requested_reference_space_types(String p_requested_reference_space_types) = 0;
	virtual String get_requested_reference_space_types() const = 0;
	virtual String get_reference_space_type() const = 0;
	virtual ARVRPositionalTracker *get_controller(int p_controller_id) const = 0;
	virtual String get_visibility_state() const = 0;
	virtual PoolVector3Array get_bounds_geometry() const = 0;
};

#endif // WEBXR_INTERFACE_H
