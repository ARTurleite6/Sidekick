package main

import "base:runtime"
import "core:c"
import "core:fmt"
import "vendor:glfw"

Window :: struct {
	handle: glfw.WindowHandle,
	io:     IO,
}

window_init :: proc(w: ^Window) -> (ok: bool) {
	glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, 4)
	glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, 1)
	w.handle = glfw.CreateWindow(800, 600, "Sidekick", nil, nil)
	if w.handle == nil {
		fmt.eprintfln("Error creating window: %s", glfw.GetError())
		return
	}

	glfw.SetWindowUserPointer(w.handle, w)

	glfw.SetKeyCallback(w.handle, key_callback)

	glfw.MakeContextCurrent(w.handle)

	ok = true
	return
}

window_destroy :: proc(w: ^Window) {
	glfw.DestroyWindow(w.handle)
}

window_update :: proc(w: ^Window) {
	io_init(&w.io)
	glfw.PollEvents()
}

window_swap_buffers :: proc(w: ^Window) {
	glfw.SwapBuffers(w.handle)
}

@(require_results)
window_should_close :: proc(w: Window) -> bool {
	return bool(glfw.WindowShouldClose(w.handle))
}

@(private = "file")
key_callback :: proc "c" (window: glfw.WindowHandle, key, scancode, action, mods: c.int) {
	w := (^Window)(glfw.GetWindowUserPointer(window))

	process_key :: proc "contextless" (io: ^IO, key: Key, action: c.int) {
		switch action {
		case glfw.PRESS:
			io.key_pressed += {key}
			io.key_down += {key}
		case glfw.RELEASE:
			io.key_released += {key}
			io.key_down -= {key}
		case glfw.REPEAT:
		}
	}

	switch key {
	case glfw.KEY_LEFT_CONTROL:
		process_key(&w.io, .Left_Ctrl, action)
	case glfw.KEY_RIGHT_CONTROL:
		process_key(&w.io, .Right_Ctrl, action)
	case glfw.KEY_LEFT_ALT:
		process_key(&w.io, .Left_Alt, action)
	case glfw.KEY_RIGHT_ALT:
		process_key(&w.io, .Right_Alt, action)
	case glfw.KEY_LEFT_SHIFT:
		process_key(&w.io, .Left_Shift, action)
	case glfw.KEY_RIGHT_SHIFT:
		process_key(&w.io, .Right_Shift, action)

	case glfw.KEY_ESCAPE:
		process_key(&w.io, .Escape, action)
	case glfw.KEY_TAB:
		process_key(&w.io, .Tab, action)
	case glfw.KEY_SPACE:
		process_key(&w.io, .Space, action)
	case glfw.KEY_DELETE:
		process_key(&w.io, .Delete, action)
	case glfw.KEY_INSERT:
		process_key(&w.io, .Insert, action)

	case glfw.KEY_APOSTROPHE:
		process_key(&w.io, .Apostrophe, action)
	case glfw.KEY_COMMA:
		process_key(&w.io, .Comma, action)
	case glfw.KEY_MINUS:
		process_key(&w.io, .Minus, action)
	case glfw.KEY_PERIOD:
		process_key(&w.io, .Period, action)
	case glfw.KEY_SLASH:
		process_key(&w.io, .Slash, action)
	case glfw.KEY_SEMICOLON:
		process_key(&w.io, .Semicolon, action)
	case glfw.KEY_EQUAL:
		process_key(&w.io, .Equal, action)
	case glfw.KEY_BACKSLASH:
		process_key(&w.io, .Backslash, action)
	case glfw.KEY_LEFT_BRACKET:
		process_key(&w.io, .Bracket_Left, action)
	case glfw.KEY_RIGHT_BRACKET:
		process_key(&w.io, .Bracket_Right, action)
	case glfw.KEY_GRAVE_ACCENT:
		process_key(&w.io, .Grave_Accent, action)

	case glfw.KEY_HOME:
		process_key(&w.io, .Home, action)
	case glfw.KEY_END:
		process_key(&w.io, .End, action)
	case glfw.KEY_PAGE_UP:
		process_key(&w.io, .Page_Up, action)
	case glfw.KEY_PAGE_DOWN:
		process_key(&w.io, .Page_Down, action)

	case 'A' ..= 'Z':
		process_key(&w.io, .A + Key(key - 'A'), action)
	case '0' ..= '9':
		process_key(&w.io, .Key_0 + Key(key - '0'), action)

	case glfw.KEY_KP_0 ..= glfw.KEY_KP_9:
		process_key(&w.io, .Numpad_0 + Key(key - glfw.KEY_KP_0), action)

	case glfw.KEY_KP_DIVIDE:
		process_key(&w.io, .Numpad_Divide, action)
	case glfw.KEY_KP_MULTIPLY:
		process_key(&w.io, .Numpad_Multiply, action)
	case glfw.KEY_KP_SUBTRACT:
		process_key(&w.io, .Numpad_Subtract, action)
	case glfw.KEY_KP_ADD:
		process_key(&w.io, .Numpad_Add, action)
	case glfw.KEY_KP_DECIMAL:
		process_key(&w.io, .Numpad_Decimal, action)

	case glfw.KEY_UP:
		process_key(&w.io, .Up, action)
	case glfw.KEY_DOWN:
		process_key(&w.io, .Down, action)
	case glfw.KEY_LEFT:
		process_key(&w.io, .Left, action)
	case glfw.KEY_RIGHT:
		process_key(&w.io, .Right, action)
	}
}

