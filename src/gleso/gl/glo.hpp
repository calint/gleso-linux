#pragma once
#include"../../platform.hpp"
#include"../metrics.hpp"
#include"gl.hpp"
#include"texture.hpp"

namespace gleso{namespace gl{

	class glo{
		class texture*tex{nullptr};
	#ifdef GLESO_EMBEDDED
		vector<GLfloat>vertices;
		vector<GLfloat>texture_coords;
		vector<GLfloat>colors;
	#else
		GLuint glid_buffer_vertices{0};
		GLuint glid_buffer_texture_coords{0};
		GLuint glid_buffer_colors{0};
	#endif
	public:
		static glo instance;

		inline glo(){metric.glo_count++;}
		inline virtual~glo(){metric.glo_count--;}
		inline glo&set_texture(texture*t){tex=t;return*this;}
		inline const texture&textureref()const{return*tex;}
		inline texture&get_texture_for_update()const{return*tex;}
		int load(){// called when context is (re)created
	#ifdef GLESO_EMBEDDED
			vertices=make_vertices();
			texture_coords=make_texture_coords();
			colors=make_colors();
	#else
			const vector<GLfloat>v1=make_vertices();
			glGenBuffers(1,&glid_buffer_vertices);
	//		p("    vertices buffer glid=%d\n",glid_buffer_vertices);
			glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
			glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v1.size()*sizeof(GLfloat)),v1.data(),GL_STATIC_DRAW);
			shader::check_gl_error("load vertices");

			const vector<GLfloat>v2=make_texture_coords();
			if(!v2.empty()){
				glGenBuffers(1,&glid_buffer_texture_coords);
	//			p("    texture coords buffer glid=%d\n",glid_buffer_texture_coords);
				glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_texture_coords);
				glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v2.size()*sizeof(GLfloat)),v2.data(),GL_STATIC_DRAW);
				shader::check_gl_error("load texture coords");
			}

			const vector<GLfloat>v3=make_colors();
			if(!v3.empty()){
				glGenBuffers(1,&glid_buffer_colors);
	//			p("    colors buffer glid=%d\n",glid_buffer_colors);
				glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_colors);
				glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v3.size()*sizeof(GLfloat)),v3.data(),GL_STATIC_DRAW);
				shader::check_gl_error("load colors");
			}
			shader::check_gl_error("load texture coords");
	#endif
			return 0;
		}
		void render()const{
			glEnableVertexAttribArray(gl::apos);
	#ifdef GLESO_EMBEDDED
			glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,&vertices[0]);
	#else
			glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
			glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,0);
	#endif
			if(tex){
				glEnableVertexAttribArray(gl::auv);
	#ifdef GLESO_EMBEDDED
				glVertexAttribPointer(gl::auv,2,GL_FLOAT,GL_FALSE,0,&texture_coords[0]);
	#else
				glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_texture_coords);
				glVertexAttribPointer(gl::auv,2,GL_FLOAT,GL_FALSE,0,0);
	#endif
				tex->enable_for_gl_draw();
			}

	#ifdef GLESO_EMBEDDED
			if(!colors.empty()){
				glEnableVertexAttribArray(gl::argba);
				glVertexAttribPointer(gl::argba,4,GL_FLOAT,GL_FALSE,0,&colors[0]);
	#else
			if(glid_buffer_colors){
				glEnableVertexAttribArray(gl::argba);
				glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_colors);
				glVertexAttribPointer(gl::argba,4,GL_FLOAT,GL_FALSE,0,0);
	#endif
			}

			gldraw();

			glDisableVertexAttribArray(gl::apos);
			if(tex)glDisableVertexAttribArray(gl::auv);
	#ifdef GLESO_EMBEDDED
			if(!colors.empty())glDisableVertexAttribArray(gl::argba);
	#else
			if(gl::argba)glDisableVertexAttribArray(gl::argba);
	#endif
		}
	protected:
		virtual vector<GLfloat>make_vertices()const{
			vector<GLfloat>v{0,1,  -1,-1,  1,-1};
			return v;
		}
		virtual vector<GLfloat>make_texture_coords()const{return vector<GLfloat>();}
		virtual vector<GLfloat>make_colors()const{return vector<GLfloat>();}
		virtual void gldraw()const{glDrawArrays(GL_TRIANGLES,0,3);}
	};
	glo glo::instance=glo{};

}}
