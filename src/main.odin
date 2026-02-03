package main

import os "core:os/os2"
import gl "vendor:OpenGL"
import "vendor:glfw"

main :: proc() {
	glfw.Init()
	defer glfw.Terminate()

	window: Window
	if !window_init(&window) {
		return
	}
	defer window_destroy(&window)

	gl.load_up_to(4, 1, glfw.gl_set_proc_address)

	vertex_content, _ := os.read_entire_file_from_path("shaders/simple.vert", context.allocator)
	vertex_shader, vertex_ok := gl.compile_shader_from_source(
		string(vertex_content),
		.VERTEX_SHADER,
	)
	assert(vertex_ok)
	defer gl.DeleteShader(vertex_shader)

	fragment_content, _ := os.read_entire_file_from_path("shaders/simple.frag", context.allocator)
	fragment_shader, fragment_ok := gl.compile_shader_from_source(
		string(fragment_content),
		.FRAGMENT_SHADER,
	)
	assert(fragment_ok)
	defer gl.DeleteShader(fragment_shader)

	program, program_ok := gl.create_and_link_program({vertex_shader, fragment_shader})
	assert(program_ok)
	defer gl.DeleteProgram(program)

	vao: u32
	gl.GenVertexArrays(1, &vao)
	defer gl.DeleteVertexArrays(1, &vao)
	gl.BindVertexArray(vao)

	running := true
	for running {
		window_update(&window)

		if window_should_close(window) {
			running = false
		}

		gl.Clear(gl.COLOR_BUFFER_BIT)

		gl.UseProgram(program)
		gl.DrawArrays(gl.TRIANGLES, 0, 3)

		window_swap_buffers(&window)
	}
}

