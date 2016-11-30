#pragma once
#include"../../platform.hpp"
#include"gl.hpp"
#include"shader.hpp"

namespace gleso{namespace gl{

	class texture{
	public:
		inline texture(){metric.texture_count++;}
		~texture(){metric.texture_count--;}
		inline void load(){
			glGenTextures(1,&glid_texture);
	//		p("    texture  glid=%d\n",glid_texture);
			refresh_from_data();
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			gl::shader::check_gl_error();
		}
		inline void enable_for_gl_draw(){
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glid_texture,0);
			glBindTexture(GL_TEXTURE_2D,glid_texture);
		}
		inline void refresh_from_data(){
			glBindTexture(GL_TEXTURE_2D,glid_texture);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)texels());
		}
		inline GLubyte*texels(){
			return data;
		}
	private:
		GLuint glid_texture{0};
		GLsizei width=2;
		GLsizei height=2;
		GLubyte data[16]={
			0xff,0x00,0x00,0xff,   0x00,0xff,0x00,0xff,
			0x00,0x00,0xff,0xff,   0xff,0xff,0x00,0xff,
		};
	//	GLuint data[16]={
	//		0xffff0000,   0xff00ff00,
	//		0xff0000ff,   0xffffff00,
	//	};
	public:
		static texture instance;
	};
	texture texture::instance=texture{};

}}
