#pragma once
#include"../../platform.hpp"
#include"gl.hpp"
#include"../metrics.hpp"

namespace gleso{namespace gl{

	class shader{
		GLint program{0};
		GLint apos{0};
		GLint auv{0};
		GLint argba{0};
		GLint umtx_mw{0};
		GLint umtx_wvp{0};
		GLint utex{0};
	public:
		inline shader(){metric.shader_count++;}

		inline virtual~shader(){
			metric.shader_count--;
			if(program){
				glDeleteProgram(program);
				program=0;
			}
		}

		static void print_gl_string(const char *name,const GLenum s){
			const char*v=(const char*)glGetString(s);
			p("%s=%s\n",name,v);
		}

		static const char*get_gl_error_string(const GLenum error){
			const char*str;
			switch(error){
				case GL_NO_ERROR:str="GL_NO_ERROR";break;
				case GL_INVALID_ENUM:str="GL_INVALID_ENUM";break;
				case GL_INVALID_VALUE:str="GL_INVALID_VALUE";break;
				case GL_INVALID_OPERATION:str="GL_INVALID_OPERATION";break;
				#if defined __gl_h_ || defined __gl3_h_
				case GL_OUT_OF_MEMORY:str="GL_OUT_OF_MEMORY";break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:str="GL_INVALID_FRAMEBUFFER_OPERATION";break;
				#endif
				#if defined __gl_h_
				case GL_STACK_OVERFLOW:str="GL_STACK_OVERFLOW";break;
				case GL_STACK_UNDERFLOW:str="GL_STACK_UNDERFLOW";break;
				case GL_TABLE_TOO_LARGE:str="GL_TABLE_TOO_LARGE";break;
				#endif
				default:str="(ERROR: Unknown Error Enum)";break;
			}
			return str;
		}
		static void check_gl_error(const char*op=""){
			bool err=false;
			for(GLenum error=glGetError();error;error=glGetError()){
				p("!!! %s   glerror %x   %s\n",op,error,get_gl_error_string(error));
				err=true;
			}
			if(err)throw"detected gl error";
		}
		static const char*get_shader_name_for_type(GLenum shader_type){
			switch(shader_type){
			case GL_VERTEX_SHADER:return"vertex";
			case GL_FRAGMENT_SHADER:return"fragment";
			default:return"unknown";
			}
		}
		void load(){
			load_program(vertex_shader_source(),fragment_shader_source());
			bind();
		}
		void viewport(const int wi,const int hi){
			glViewport(0,0,wi,hi);
		}
		void use_program(){
			if(active_program==program)return;
	//		p(" activating program  %d\n",glid_program);
			glUseProgram(program);
			prepare_gl_for_render();
			active_program=program;
		}

	private:
		void load_program(const char*vertex_shader_source,const char*fragment_shader_source){
			program=glCreateProgram();
			if(!program)throw"cannot create program";
	//		p("    program glid=%d\n",glid_program);
			glAttachShader(program,load_shader(GL_VERTEX_SHADER,vertex_shader_source));
			glAttachShader(program,load_shader(GL_FRAGMENT_SHADER,fragment_shader_source));
			glLinkProgram(program);
			GLint linkStatus=GL_FALSE;
			glGetProgramiv(program,GL_LINK_STATUS,&linkStatus);
			if(linkStatus)return;
			GLint info_len{0};
			glGetProgramiv(program,GL_INFO_LOG_LENGTH,&info_len);
			unique_ptr<GLchar>info=make_unique<GLchar>(info_len);
			glGetProgramInfoLog(program,info_len,NULL,info.get());
			p("!!! could not link program:\n%s\n",info.get());
			glDeleteProgram(program);
			program=0;
			throw"error while linking";
		}
		static GLuint load_shader(const GLenum shader_type,const char*source){
			const GLuint shader=glCreateShader(shader_type);
	//		p("    %s shader glid=%d\n",get_shader_name_for_type(shader_type),shader);
			glShaderSource(shader,1,&source,NULL);
			glCompileShader(shader);
			GLint compiled{0};
			glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
			if(compiled)return shader;
			GLint info_len{0};
			glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&info_len);
			unique_ptr<GLchar>info_buf=make_unique<GLchar>(info_len);
			glGetShaderInfoLog(shader,info_len,NULL,info_buf.get());
			p("!!! could not compile %s shader:\n%s\n",get_shader_name_for_type(shader_type),info_buf.get());
			glDeleteShader(shader);
			throw"could not compile shader";
		}
	protected:
		inline virtual const GLchar*vertex_shader_source()const{return shader_source_vertex;}
		inline virtual const GLchar*fragment_shader_source()const{return shader_source_fragment;}
	const GLchar*shader_source_vertex=R"(
	#version 100
	uniform mat4 umtx_mw;// model-world matrix
	uniform mat4 umtx_wvp;// view-projection matrix
	attribute vec4 apos;// vertices
	attribute vec2 auv;// texture coords
	attribute vec4 argba;// colors
	varying vec2 vuv;
	varying vec4 vrgba;
	void main(){
		gl_Position=umtx_wvp*umtx_mw*apos;
		vuv=auv;
		vrgba=argba;
	}
	)";
	const GLchar*shader_source_fragment=R"(
	#version 100
	uniform sampler2D utex;
	varying mediump vec2 vuv;
	varying mediump vec4 vrgba;
	void main(){
		mediump vec4 tx=texture2D(utex,vuv);
		gl_FragColor=tx+vrgba;
	}
	)";

		inline GLint get_attribute_location(const char*name){return glGetAttribLocation(program,name);}
		inline GLint get_uniform_location(const char*name){return glGetUniformLocation(program,name);}
		#define A(x,y)if((x=get_attribute_location(y))==-1){p("shader: cannot find attribute %s\n",y);throw"error";};
		#define U(x,y)if((x=get_uniform_location(y))==-1){p("shader: cannot find uniform %s\n",y);throw"error";}
//
//		inline int attribute(const char*name){
//			if((x=get_attribute_location(y))==-1){p("shader: cannot find attribute %s\n",y);throw"error";
//		}

		inline virtual void bind(){
			A(apos,"apos");
			A(auv,"auv");
			A(argba,"argba");
			U(umtx_mw,"umtx_mw");
			U(umtx_wvp,"umtx_wvp");
			U(utex,"utex");
		}

		inline virtual void prepare_gl_for_render(){
			gl::umtx_mw=umtx_mw;
			gl::umtx_wvp=umtx_wvp;
			gl::utex=utex;
			gl::apos=apos;
			gl::auv=auv;
			gl::argba=argba;
		}

	public:
		static shader instance;
	};
	shader shader::instance=shader{};
}}
