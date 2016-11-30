#pragma once
#include"../../include.hpp"

namespace gleso{
	class glob;
	vector<glob*>globs;

	namespace gl{
		class shader;
		class glo;
		class texture;
		class a_camera;

		shader*active_shader;
		GLint active_program;
		vector<shader*>shaders;
		vector<texture*>textures;
		vector<glo*>glos;
		vector<a_camera*>cameras;
		a_camera*active_camera;
	}
}
