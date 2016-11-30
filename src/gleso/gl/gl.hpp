#pragma once
#include<vector>
#include<memory>
using namespace std;

namespace gleso{
	class glob;

	namespace gl{
		class shader;
		class glo;
		class texture;
		class a_camera;

		// - - glsl bindings - - - - - - -
		GLint apos;// vec2 vertex coords x,y
		GLint auv;// vec2 texture coords x,y
		GLint argba;// vec4 colors
		GLint umtx_mw;// mat4 model->world matrix
		GLint umtx_wvp;// mat4 world->view->projection matrix
		GLint utex;// texture sampler
		// - - - - - - - - - - - - - - - -
		shader*active_shader;
		GLint active_program;
		vector<shader*>shaders;
		vector<texture*>textures;
		vector<glo*>glos;
		vector<glob*>globs;
		vector<a_camera*>cameras;
		a_camera*active_camera;
	}
}
