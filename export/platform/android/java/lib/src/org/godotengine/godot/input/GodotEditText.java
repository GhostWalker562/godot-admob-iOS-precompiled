/*************************************************************************/
/*  GodotEditText.java                                                   */
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

package org.godotengine.godot.input;

import org.godotengine.godot.*;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.text.InputFilter;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

import java.lang.ref.WeakReference;

public class GodotEditText extends EditText {
	// ===========================================================
	// Constants
	// ===========================================================
	private final static int HANDLER_OPEN_IME_KEYBOARD = 2;
	private final static int HANDLER_CLOSE_IME_KEYBOARD = 3;

	// ===========================================================
	// Fields
	// ===========================================================
	private GodotView mView;
	private GodotTextInputWrapper mInputWrapper;
	private EditHandler sHandler = new EditHandler(this);
	private String mOriginText;
	private int mMaxInputLength = Integer.MAX_VALUE;
	private boolean mMultiline = false;

	private static class EditHandler extends Handler {
		private final WeakReference<GodotEditText> mEdit;
		public EditHandler(GodotEditText edit) {
			mEdit = new WeakReference<>(edit);
		}

		@Override
		public void handleMessage(Message msg) {
			GodotEditText edit = mEdit.get();
			if (edit != null) {
				edit.handleMessage(msg);
			}
		}
	}

	// ===========================================================
	// Constructors
	// ===========================================================
	public GodotEditText(final Context context) {
		super(context);
		this.initView();
	}

	public GodotEditText(final Context context, final AttributeSet attrs) {
		super(context, attrs);
		this.initView();
	}

	public GodotEditText(final Context context, final AttributeSet attrs, final int defStyle) {
		super(context, attrs, defStyle);
		this.initView();
	}

	protected void initView() {
		this.setPadding(0, 0, 0, 0);
		this.setImeOptions(EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_ACTION_DONE);
	}

	public boolean isMultiline() {
		return mMultiline;
	}

	private void handleMessage(final Message msg) {
		switch (msg.what) {
			case HANDLER_OPEN_IME_KEYBOARD: {
				GodotEditText edit = (GodotEditText)msg.obj;
				String text = edit.mOriginText;
				if (edit.requestFocus()) {
					edit.removeTextChangedListener(edit.mInputWrapper);
					setMaxInputLength(edit);
					edit.setText("");
					edit.append(text);
					if (msg.arg2 != -1) {
						edit.setSelection(msg.arg1, msg.arg2);
						edit.mInputWrapper.setSelection(true);
					} else {
						edit.mInputWrapper.setSelection(false);
					}

					int inputType = InputType.TYPE_CLASS_TEXT;
					if (edit.isMultiline()) {
						inputType |= InputType.TYPE_TEXT_FLAG_MULTI_LINE;
					}
					edit.setInputType(inputType);

					edit.mInputWrapper.setOriginText(text);
					edit.addTextChangedListener(edit.mInputWrapper);
					final InputMethodManager imm = (InputMethodManager)mView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
					imm.showSoftInput(edit, 0);
				}
			} break;

			case HANDLER_CLOSE_IME_KEYBOARD: {
				GodotEditText edit = (GodotEditText)msg.obj;

				edit.removeTextChangedListener(mInputWrapper);
				final InputMethodManager imm = (InputMethodManager)mView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
				imm.hideSoftInputFromWindow(edit.getWindowToken(), 0);
				edit.mView.requestFocus();
			} break;
		}
	}

	private void setMaxInputLength(EditText p_edit_text) {
		InputFilter[] filters = new InputFilter[1];
		filters[0] = new InputFilter.LengthFilter(this.mMaxInputLength);
		p_edit_text.setFilters(filters);
	}

	// ===========================================================
	// Getter & Setter
	// ===========================================================
	public void setView(final GodotView view) {
		this.mView = view;
		if (mInputWrapper == null)
			mInputWrapper = new GodotTextInputWrapper(mView, this);
		this.setOnEditorActionListener(mInputWrapper);
		view.requestFocus();
	}

	// ===========================================================
	// Methods for/from SuperClass/Interfaces
	// ===========================================================
	@Override
	public boolean onKeyDown(final int keyCode, final KeyEvent keyEvent) {
		/* Let SurfaceView get focus if back key is input. */
		// pass event to godot in special cases
		if (needHandlingInGodot(keyCode, keyEvent) && mView.getInputHandler().onKeyDown(keyCode, keyEvent)) {
			return true;
		} else {
			return super.onKeyDown(keyCode, keyEvent);
		}
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent keyEvent) {
		if (needHandlingInGodot(keyCode, keyEvent) && mView.getInputHandler().onKeyUp(keyCode, keyEvent)) {
			return true;
		} else {
			return super.onKeyUp(keyCode, keyEvent);
		}
	}

	private boolean needHandlingInGodot(int keyCode, KeyEvent keyEvent) {
		boolean isArrowKey = keyCode == KeyEvent.KEYCODE_DPAD_UP || keyCode == KeyEvent.KEYCODE_DPAD_DOWN ||
							 keyCode == KeyEvent.KEYCODE_DPAD_LEFT || keyCode == KeyEvent.KEYCODE_DPAD_RIGHT;
		boolean isModifiedKey = keyEvent.isAltPressed() || keyEvent.isCtrlPressed() || keyEvent.isSymPressed() ||
								keyEvent.isFunctionPressed() || keyEvent.isMetaPressed();
		return isArrowKey || keyCode == KeyEvent.KEYCODE_TAB || KeyEvent.isModifierKey(keyCode) ||
				isModifiedKey;
	}

	// ===========================================================
	// Methods
	// ===========================================================
	public void showKeyboard(String p_existing_text, boolean p_multiline, int p_max_input_length, int p_cursor_start, int p_cursor_end) {
		int maxInputLength = (p_max_input_length <= 0) ? Integer.MAX_VALUE : p_max_input_length;
		if (p_cursor_start == -1) { // cursor position not given
			this.mOriginText = p_existing_text;
			this.mMaxInputLength = maxInputLength;
		} else if (p_cursor_end == -1) { // not text selection
			this.mOriginText = p_existing_text.substring(0, p_cursor_start);
			this.mMaxInputLength = maxInputLength - (p_existing_text.length() - p_cursor_start);
		} else {
			this.mOriginText = p_existing_text.substring(0, p_cursor_end);
			this.mMaxInputLength = maxInputLength - (p_existing_text.length() - p_cursor_end);
		}

		this.mMultiline = p_multiline;

		final Message msg = new Message();
		msg.what = HANDLER_OPEN_IME_KEYBOARD;
		msg.obj = this;
		msg.arg1 = p_cursor_start;
		msg.arg2 = p_cursor_end;
		sHandler.sendMessage(msg);
	}

	public void hideKeyboard() {
		final Message msg = new Message();
		msg.what = HANDLER_CLOSE_IME_KEYBOARD;
		msg.obj = this;
		sHandler.sendMessage(msg);
	}

	// ===========================================================
	// Inner and Anonymous Classes
	// ===========================================================
}
