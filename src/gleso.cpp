#include "gleso.h"
////////////////////////////////////////////////////////////////////////
namespace metrics{
	unsigned int fps;
	unsigned int nshader;
	unsigned int ngrid;
	unsigned int nglo;
	unsigned int nglob;
	unsigned int ntexture;
	unsigned int updated_globs;
	unsigned int rendered_globs;
	float time_since_start_in_seconds;
	void log(){p("/ metrics %2.2fs – fps:%03d – shaders:%01d – textures:%01d – glos:%02d – globs:%05d – updated:%02d – rendered:%02d – grids:%02d \n",time_since_start_in_seconds,fps,nshader,ntexture,nglo,nglob,updated_globs,rendered_globs,ngrid);}
	inline void before_render(){
		updated_globs=0;
		rendered_globs=0;
	}
}
////////////////////////////////////////////////////////////////////////
class shader;
namespace gl{
	shader*shdr;
	GLint apos;// vec2 vertex coords x,y
	GLint auv;// vec2 texture coords x,y
	GLint argba;// vec4 colors
	GLint umtx_mw;// mat4 model->world matrix
	GLint umtx_wvp;// mat4 world->view->projection matrix
	GLint utex;// texture sampler

	GLint active_program;
}
////////////////////////////////////////////////////////////////////////
#include<string>
class shader{
	GLint glid_program{0};
	GLint apos{0};
	GLint auv{0};
	GLint argba{0};
	GLint umtx_mw{0};
	GLint umtx_wvp{0};
	GLint utex{0};
public:
	shader(){
		p("new shader %p\n",this);
		metrics::nshader++;
	}

	virtual~shader(){
		p("delete shader %p\n",(void*)this);
		metrics::nshader--;
		gleso_cleanup();
//		if(glid_program){glDeleteProgram(glid_program);glid_program=0;}
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
		if(gl::active_program==glid_program)return;
		p(" activating program  %d\n",glid_program);
		glUseProgram(glid_program);
		prepare_gl_for_render();
		gl::active_program=glid_program;
	}

private:
	void load_program(const char*vertex_shader_source,const char*fragment_shader_source){
		glid_program=glCreateProgram();
		if(!glid_program)throw"cannot create program";
		p("    program glid=%d\n",glid_program);
		glAttachShader(glid_program,load_shader(GL_VERTEX_SHADER,vertex_shader_source));
		glAttachShader(glid_program,load_shader(GL_FRAGMENT_SHADER,fragment_shader_source));
		glLinkProgram(glid_program);
		GLint linkStatus=GL_FALSE;
		glGetProgramiv(glid_program,GL_LINK_STATUS,&linkStatus);
		if(linkStatus)return;
		GLint info_len{0};
		glGetProgramiv(glid_program,GL_INFO_LOG_LENGTH,&info_len);
		GLchar*info=new char[info_len];
		glGetProgramInfoLog(glid_program,info_len,NULL,info);
		p("!!! could not link program:\n%s\n",info);
		delete info;
		glDeleteProgram(glid_program);
		glid_program=0;
		throw"error while linking";
	}
	static GLuint load_shader(const GLenum shader_type,const char*source){
		const GLuint shader=glCreateShader(shader_type);
		p("    %s shader glid=%d\n",get_shader_name_for_type(shader_type),shader);
		glShaderSource(shader,1,&source,NULL);
		glCompileShader(shader);
		GLint compiled{0};
		glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
		if(compiled)return shader;
		GLint info_len{0};
		glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&info_len);
		GLchar*info_buf=new GLchar[info_len];
		glGetShaderInfoLog(shader,info_len,NULL,info_buf);
		p("!!! could not compile %s shader:\n%s\n",get_shader_name_for_type(shader_type),info_buf);
		delete info_buf;
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

	inline GLint get_attribute_location(const char*name){return glGetAttribLocation(glid_program,name);}
	inline GLint get_uniform_location(const char*name){return glGetUniformLocation(glid_program,name);}
	#define A(x,y)if((x=get_attribute_location(y))==-1){p("shader: cannot find attribute %s\n",y);throw"error";};
	#define U(x,y)if((x=get_uniform_location(y))==-1){p("shader: cannot find uniform %s\n",y);throw"error";}
	virtual void bind(){
		A(apos,"apos");
		A(auv,"auv");
		A(argba,"argba");
		U(umtx_mw,"umtx_mw");
		U(umtx_wvp,"umtx_wvp");
		U(utex,"utex");
	}
	virtual void prepare_gl_for_render(){
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
shader shader::instance=shader();

////////////////////////////////////////////////
class texture{
public:
	texture(){
		p("new texture %p\n",this);
		metrics::ntexture++;
	}
	~texture(){
		p("delete texture %p\n",this);
//		glDeleteTextures(1,&glid_texture);
		metrics::ntexture--;
	}
	void load(){
		glGenTextures(1,&glid_texture);
		p("    texture  glid=%d\n",glid_texture);
		refresh_from_data();
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		shader::check_gl_error();
	}
	void enable_for_gl_draw(){
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glid_texture,0);
		glBindTexture(GL_TEXTURE_2D,glid_texture);
	}
	void refresh_from_data(){
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
texture texture::instance=texture();

#include<vector>
using std::vector;
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
	glo(){
		p("new glo %p\n",(void*)this);
		metrics::nglo++;
	}
	virtual~glo(){
		p("delete glo %p\n",this);
		metrics::nglo--;
	}
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
		p("    vertices buffer glid=%d\n",glid_buffer_vertices);
		glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
		glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v1.size()*sizeof(GLfloat)),v1.data(),GL_STATIC_DRAW);
		shader::check_gl_error("load vertices");

		const vector<GLfloat>v2=make_texture_coords();
		if(!v2.empty()){
			glGenBuffers(1,&glid_buffer_texture_coords);
			p("    texture coords buffer glid=%d\n",glid_buffer_texture_coords);
			glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_texture_coords);
			glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v2.size()*sizeof(GLfloat)),v2.data(),GL_STATIC_DRAW);
			shader::check_gl_error("load texture coords");
		}

		const vector<GLfloat>v3=make_colors();
		if(!v3.empty()){
			glGenBuffers(1,&glid_buffer_colors);
			p("    colors buffer glid=%d\n",glid_buffer_colors);
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
		vector<GLfloat>v;
		const GLfloat verts[]{0,1,  -1,-1,  1,-1};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{return vector<GLfloat>();}
	virtual vector<GLfloat>make_colors()const{return vector<GLfloat>();}
	virtual void gldraw()const{
		glDrawArrays(GL_TRIANGLES,0,3);
	}
public:
	static glo instance;
};
glo glo::instance=glo();

class p3{
public:
	inline p3():x{0},y{0},z{0}{}
//	inline p3(const floato x):x{x},y{0},z{0}{}
//	inline p3(const floato x,const floato y):x{x},y{y},z{0}{}
	inline p3(const floato x,const floato y,const floato z):x{x},y{y},z{z}{}
//	inline p3(const p3&p):x{p.x},y{p.y},z{p.z}{}
//	inline p3&x(const floato f){x_=f;return*this;}inline floato x()const{return x_;}
//	inline p3&y(const floato f){y_=f;return*this;}inline floato y()const{return y_;}
//	inline p3&z(const floato f){z_=f;return*this;}inline floato z()const{return z_;}
	inline p3&add(const p3&p,const floato dt){x+=p.x*dt;y+=p.y*dt;z+=p.z*dt;return*this;}//? simd
	p3 operator-()const{return p3{-x,-y,-z};}
	friend p3 operator+(const p3&a,const p3&b);
//private:
	floato x,y,z;
};
p3 operator+(const p3&a,const p3&b){
	return p3{a.x+b.x,a.y+b.y,a.z+b.z};
}
p3 operator-(const p3&a,const p3&b){
	return p3{a.x-b.x,a.y-b.y,a.z-b.z};
}

namespace gleso{
	floato dt;
}
class physics{
public:
	p3 p{0,0,0},dp{0,0,0},ddp{0,0,0};//position
	p3 a{0,0,0},da{0,0,0},dda{0,0,0};//angle
	p3 s{0,0,0};//scale
	floato r;//radius
	void update(){
		dp.add(ddp,gleso::dt);
		p.add(dp,gleso::dt);
		da.add(dda,gleso::dt);
		a.add(da,gleso::dt);
	}
//	inline p3&position(){return p;}
//	inline p3&pos(){return p;}
//	inline p3&dpos(){return dp;}
//	inline p3&angle(){return a;}
//	inline p3&dagl(){return da;}
//	inline p3&scale(){return s;}
//private:
	//? list of forces/velocities applied
};

class render_info{
public:
	p3 p{0,0,0},a{0,0,0},s{0,0,0};
//	inline const p3&position()const{return p;}
//	inline render_info&position(const p3&pos){p=pos;return*this;}
//	inline const p3&angle()const{return a;}
//	inline render_info&angle(const p3&agl){a=agl;return*this;}
//	inline const p3&scale()const{return s;}
//	inline render_info&scale(const p3&scl){s=scl;return*this;}
private:
};

////
// mtx funcs lifted from apple examples
#include<math.h>
static void mtxRotateZApply(floato* mtx, floato deg)
{
	// [ 0 4  8 12 ]   [ cos -sin 0  0 ]
	// [ 1 5  9 13 ] x [ sin cos  0  0 ]
	// [ 2 6 10 14 ]   [ 0   0    1  0 ]
	// [ 3 7 11 15 ]   [ 0   0    0  1 ]

	float rad = deg*float(M_PI/180.0f);

	float cosrad = cosf(rad);
	float sinrad = sinf(rad);

	float mtx00 = mtx[0];
	float mtx01 = mtx[1];
	float mtx02 = mtx[2];
	float mtx03 = mtx[3];

	mtx[ 0] = mtx[ 4]*sinrad + mtx00*cosrad;
	mtx[ 4] = mtx[ 4]*cosrad - mtx00*sinrad;

	mtx[ 1] = mtx[ 5]*sinrad + mtx01*cosrad;
	mtx[ 5] = mtx[ 5]*cosrad - mtx01*sinrad;

	mtx[ 2] = mtx[ 6]*sinrad + mtx02*cosrad;
	mtx[ 6] = mtx[ 6]*cosrad - mtx02*sinrad;

	mtx[ 3] = mtx[ 7]*sinrad + mtx03*cosrad;
	mtx[ 7] = mtx[ 7]*cosrad - mtx03*sinrad;
}
static void mtxLoadOrthographic(float* mtx,
							float left, float right,
							float bottom, float top,
							float nearZ, float farZ)
{
	//See appendix G of OpenGL Red Book

	mtx[ 0] = 2.0f / (right - left);
	mtx[ 1] = 0.0;
	mtx[ 2] = 0.0;
	mtx[ 3] = 0.0;

	mtx[ 4] = 0.0;
	mtx[ 5] = -2.0f / (top - bottom);
	mtx[ 6] = 0.0;
	mtx[ 7] = 0.0;

	mtx[ 8] = 0.0;
	mtx[ 9] = 0.0;
	mtx[10] = -2.0f / (farZ - nearZ);
	mtx[11] = 0.0;

	mtx[12] = -(right + left) / (right - left);
	mtx[13] = -(top + bottom) / (top - bottom);
	mtx[14] = -(farZ + nearZ) / (farZ - nearZ);
	mtx[15] = 1.0f;
}
static void mtxMultiply(float* ret, const float* lhs, const float* rhs)
{
	// [ 0 4  8 12 ]   [ 0 4  8 12 ]
	// [ 1 5  9 13 ] x [ 1 5  9 13 ]
	// [ 2 6 10 14 ]   [ 2 6 10 14 ]
	// [ 3 7 11 15 ]   [ 3 7 11 15 ]
	ret[ 0] = lhs[ 0]*rhs[ 0] + lhs[ 4]*rhs[ 1] + lhs[ 8]*rhs[ 2] + lhs[12]*rhs[ 3];
	ret[ 1] = lhs[ 1]*rhs[ 0] + lhs[ 5]*rhs[ 1] + lhs[ 9]*rhs[ 2] + lhs[13]*rhs[ 3];
	ret[ 2] = lhs[ 2]*rhs[ 0] + lhs[ 6]*rhs[ 1] + lhs[10]*rhs[ 2] + lhs[14]*rhs[ 3];
	ret[ 3] = lhs[ 3]*rhs[ 0] + lhs[ 7]*rhs[ 1] + lhs[11]*rhs[ 2] + lhs[15]*rhs[ 3];

	ret[ 4] = lhs[ 0]*rhs[ 4] + lhs[ 4]*rhs[ 5] + lhs[ 8]*rhs[ 6] + lhs[12]*rhs[ 7];
	ret[ 5] = lhs[ 1]*rhs[ 4] + lhs[ 5]*rhs[ 5] + lhs[ 9]*rhs[ 6] + lhs[13]*rhs[ 7];
	ret[ 6] = lhs[ 2]*rhs[ 4] + lhs[ 6]*rhs[ 5] + lhs[10]*rhs[ 6] + lhs[14]*rhs[ 7];
	ret[ 7] = lhs[ 3]*rhs[ 4] + lhs[ 7]*rhs[ 5] + lhs[11]*rhs[ 6] + lhs[15]*rhs[ 7];

	ret[ 8] = lhs[ 0]*rhs[ 8] + lhs[ 4]*rhs[ 9] + lhs[ 8]*rhs[10] + lhs[12]*rhs[11];
	ret[ 9] = lhs[ 1]*rhs[ 8] + lhs[ 5]*rhs[ 9] + lhs[ 9]*rhs[10] + lhs[13]*rhs[11];
	ret[10] = lhs[ 2]*rhs[ 8] + lhs[ 6]*rhs[ 9] + lhs[10]*rhs[10] + lhs[14]*rhs[11];
	ret[11] = lhs[ 3]*rhs[ 8] + lhs[ 7]*rhs[ 9] + lhs[11]*rhs[10] + lhs[15]*rhs[11];

	ret[12] = lhs[ 0]*rhs[12] + lhs[ 4]*rhs[13] + lhs[ 8]*rhs[14] + lhs[12]*rhs[15];
	ret[13] = lhs[ 1]*rhs[12] + lhs[ 5]*rhs[13] + lhs[ 9]*rhs[14] + lhs[13]*rhs[15];
	ret[14] = lhs[ 2]*rhs[12] + lhs[ 6]*rhs[13] + lhs[10]*rhs[14] + lhs[14]*rhs[15];
	ret[15] = lhs[ 3]*rhs[12] + lhs[ 7]*rhs[13] + lhs[11]*rhs[14] + lhs[15]*rhs[15];}
///////////////////////////////////////////////////////

class m4{
public:
	floato c[16]{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
	inline m4&load_translate(const p3&p){
		// [ 0 4  8  x ]
		// [ 1 5  9  y ]
		// [ 2 6 10  z ]
		// [ 3 7 11 15 ]
		c[ 0]=c[ 5]=c[10]=c[15]=1;
		c[ 1]=c[ 2]=c[ 3]=c[ 4]=
		c[ 6]=c[ 7]=c[ 8]=c[ 9]=
		c[11]=0.0;
		c[12]=p.x;
		c[13]=p.y;
		c[14]=p.z;
		return*this;
	}
	inline m4&append_rotation_about_z_axis(const floato degrees){
		mtxRotateZApply(c,degrees);
		return*this;
	}
	inline m4&append_scaling(const p3&scale){
		// [ 0 4  8 12 ]   [ x 0 0 0 ]
		// [ 1 5  9 13 ] x [ 0 y 0 0 ]
		// [ 2 6 10 14 ]   [ 0 0 z 0 ]
		// [ 3 7 11 15 ]   [ 0 0 0 1 ]
		c[ 0]*=scale.x;
		c[ 4]*=scale.y;
		c[ 8]*=scale.z;

		c[ 1]*=scale.x;
		c[ 5]*=scale.y;
		c[ 9]*=scale.z;

		c[ 2]*=scale.x;
		c[ 6]*=scale.y;
		c[10]*=scale.z;

		c[ 3]*=scale.x;
		c[ 7]*=scale.y;
		c[11]*=scale.z;
		return*this;
	}
	inline m4&load_ortho_projection(floato left,floato right,floato bottom,floato top,floato nearZ,floato farZ){
		mtxLoadOrthographic(c,left,right,bottom,top,nearZ,farZ);
		return*this;
	}
	inline const floato*array()const{return c;}
};
class grid;
class glob{
public:
	physics phy;// current physics state
	glo*gl{nullptr};// ref to gl renderable
	grid*grid_that_updates_this_glob{nullptr};//managed by grid
	glob(){
		p("new glob %p\n",(void*)this);
		metrics::nglob++;
		phy.r=.1;
		phy.s={phy.r,phy.r,phy.r};
	}
	virtual~glob(){
		p("delete glob %p\n",(void*)this);
		metrics::nglob--;
	}
//	inline glob&glo_ref(const class glo*g){glo=g;return*this;}
//	inline physics&phys(){return phy;}
//	inline const p3&scale()const{return scal;}
//	inline glob&scale(const p3&scale){scal=scale;return*this;}
	void render(){
		metrics::rendered_globs++;
		if(!gl)return;
//		return;
		render_info=render_info_next;
		matrix_model_world.load_translate(render_info.p);
		matrix_model_world.append_rotation_about_z_axis(render_info.a.z);
		matrix_model_world.append_scaling(render_info.s);
		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,matrix_model_world.array());
		gl->render();
	}
	void update(){
		metrics::updated_globs++;
//		phy_prv=phy;
//		phy=phy_nxt;
//		phy.s=scal;
		phy.update();
		on_update();
		render_info_next.p=phy.p;
		render_info_next.a=phy.a;
		render_info_next.s=phy.s;
	}
	virtual void on_update(){}
private:
	m4 matrix_model_world;
	class render_info render_info;// info for opengl rendering
	class render_info render_info_next;// next renderinfo, updated during render
//	physics phy_prv;// previous physics state
//	physics phy_nxt;// next physics state
};

class camera:public glob{
//	m4 mtx_wvp;// world->view->projection
public:
	int screen_width{320},screen_height{240};
	camera(){
		gl=&glo::instance;
		//phy.da.z=30;
	}
	void pre_render(){
		gl::shdr->use_program();
		glClearColor(floato{.5},0,floato{.5},1);
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		m4 wvp,wv,p;
		wv.load_translate(-phy.p);
		wv.append_rotation_about_z_axis(-phy.a.z);
		const float aspect_ratio=floato(screen_height)/floato(screen_width);
		p.load_ortho_projection(-1,1,aspect_ratio,-aspect_ratio,0,1);
		mtxMultiply(wvp.c, p.c, wv.c);

		glUniformMatrix4fv(GLint(gl::umtx_wvp),1,false,wvp.c);
	}
//	virtual void on_update(){
//		if(phy.p.x>1)
//			phy.dp.x=-1;
//		else if(phy.p.x<-1)
//			phy.dp.x=1;
//	}
//	inline const m4&matrix_world_view_projection()const{return mtx_wvp;}
};

/*-----------------------------
     __    _       __     __
    /  \   |      /  \   /  \
   /       |     |    | |
   |  ___  |     |    |  \__
   |    |  |     |    |     \
    \___/  |___|  \__/   \__/
 
------------------------------
"defglo" */
class glo_square_xy:public glo{
	virtual vector<GLfloat>make_vertices()const{
		const static GLfloat verts[]={-1,1, -1,-1, 1,-1, 1,1};
		vector<GLfloat>v;//{-1,1, -1,-1, 1,-1, 1,1};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		vector<GLfloat>v;
//		const static GLfloat verts[]={-1,1, -1,-1, 1,-1, 1,1};
//		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	inline virtual void gldraw()const{
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
	}
};
class glo_circle_xy:public glo{
	int nvertices;
public:
	glo_circle_xy():nvertices(1+12+1){}
protected:
	virtual vector<GLfloat>make_vertices()const{
		vector<GLfloat>v;
		v.push_back(0);//x
		v.push_back(0);//y
		floato rad=0;
		const floato drad=2*float(M_PI/12);
		for(int i=1;i<=nvertices;i++){
			const floato x=cosf(rad);
			const floato y=sinf(rad);
			rad+=drad;
			v.push_back(x);
			v.push_back(y);
		}
		return v;
	}
	virtual void gldraw()const{
		glDrawArrays(GL_TRIANGLE_FAN,0,nvertices);
	}
};
class glo_square_xyuv:public glo{
public:
	glo_square_xyuv(){
		set_texture(&texture::instance);
	}
	virtual vector<GLfloat>make_vertices()const{
		vector<GLfloat>v;
		const static GLfloat verts[]{
		//	 x  y
			-1, 1,
			-1,-1,
			 1,-1,
			 1, 1
		};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		vector<GLfloat>v;
		const static GLfloat verts[]{
		//	u v
			0,1,
			0,0,
			1,0,
			1,1
		};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	inline virtual void gldraw()const{
//		texels_rgb[0]++;
//		texels_rgb[1]++;
		get_texture_for_update().refresh_from_data();
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
	}
	virtual vector<GLfloat>make_colors()const{
		return vector<GLfloat>();
	}
};
class glo_square_xyuvrgba:public glo_square_xyuv{
public:
//	glo_square_xyuvrgba(){
//		texture(nullptr);
//	}
	virtual vector<GLfloat>make_colors()const{
		vector<GLfloat>v;
		const GLfloat f[]{
		//   R  G  B A
			 0, 0, 0,1,
			 1, 1, 1,1,
			 0, 0, 0,1,
			-1,-1,-1,1,
		};
		v.assign(f,f+sizeof(f)/sizeof(GLfloat));
		return v;
	}
};
class glo_grid:public glo{
	virtual vector<GLfloat>make_vertices()const{
		vector<GLfloat>v;
		const static GLfloat verts[]{
		//	 x  y
			-1, 1,
			-1,-1,
			 1,-1,
			 1, 1
		};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_colors()const{
		vector<GLfloat>v;
		const GLfloat f[]{
		//   R  G  B A
			 1, 1, 1, 1,
			 1, 1, 1, 1,
			 1, 1, 1, 1,
			 1, 1, 1, 1,
		};
		v.assign(f,f+sizeof(f)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		vector<GLfloat>v;
		return v;
	}
	inline virtual void gldraw()const{
		glDrawArrays(GL_LINE_LOOP,0,4);
	}
public:
	static glo_grid instance;
};
glo_grid glo_grid::instance=glo_grid();

//------------------------------------------------------------------------------
#include<list>
using std::list;
#include<algorithm>
#define foreach(c,f)std::for_each(c.begin(),c.end(),f)
class grid{
	p3 po;
	float s;
	list<glob*>ls;
	list<glob*>lsmx;
	grid*grds[8]{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
	const size_t splitthresh=64;
	const int subgridlevels=5;
public:
	grid(const floato size,const p3&p=p3{}):po(p),s(size){
		metrics::ngrid++;
	}
	~grid(){
		metrics::ngrid--;
		clear();
	}
	void clear(){
		ls.clear();
		lsmx.clear();
		for(auto&g:grds)
			if(g){
				g->clear();
				delete g;//? recycle
				g=nullptr;
			}
	}
	void addall(const list<glob*>&ls){
		for(auto g:ls){
			g->grid_that_updates_this_glob=nullptr;
			putif(g,g->phy.p,g->phy.r);
		}
		splitif(subgridlevels);//? splititonthefly
		//? ifallglobswhereaddedtoallsubgrids,stoprecurtion
	}
	void update_globs(){//! multicore
//		p(" update grid %p\n",(void*)this);
		for(auto g:ls){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->update();
		}
		for(auto g:lsmx){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->update();
		}
		for(auto&g:grds)
			if(g)
				g->update_globs();
	}
	void render_globs(){
		for(auto g:ls){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->render();
		}
		for(auto g:lsmx){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->render();
		}
		for(auto&g:grds)
			if(g)
				g->render_globs();

	}
	void render_outline(){
		bool rendered=false;
		for(auto gr:grds){
			if(!gr)
				continue;
			gr->render_outline();
			rendered=true;
		}
		if(rendered)return;
//		if(ls.empty())return;
		m4 m;
		m.load_translate(po);
		m.append_scaling(p3{s,s,s});
		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,m.array());
		glo_grid::instance.render();
	}
//	void coldet(){
//		if(!ls.empty()){
//			auto i1=ls.begin();
//			while(true){
//				auto i2=ls.rbegin();
//				if(*i1==*i2)
//					break;
//				glob&g1=*(*i1);
//				do{	glob&g2=*(*i2);
//					g1.coldet(g2);
//					i2++;
//				}while(*i1!=*i2);
//				i1++;
//			}
//			if(!lsmx.empty())
//				for(auto g1:ls)
//					for(auto g2:lsmx)
//						g1->coldet(*g2);
//		}
//		for(auto g:grds)
//			if(g)
//				g->coldet();
//	}
//	void culldraw(const bvol&bv){
//		const int c=bv.cull(po,s*1.41f);//? radius
//		if(c){
//			metrics::gridsculled++;
//			return;
//		}
//		metrics::gridsrend++;
//		for(auto g:ls){glPushMatrix();g->culldraw(bv);glPopMatrix();}
//		for(auto g:lsmx){glPushMatrix();g->culldraw(bv);glPopMatrix();}
//		for(auto&g:grds)
//			if(g)
//				g->culldraw(bv);
//	}
private:
	inline bool putif(glob*g,const p3&p,const floato r){
		if((p.x+s+r)<po.x)return false;
		if((p.x-s-r)>po.x)return false;
		if((p.z+s+r)<po.z)return false;
		if((p.z-s-r)>po.z)return false;
		if((p.y+s+r)<po.y)return false;
		if((p.y-s-r)>po.y)return false;
//		if(g->iscolmx()){
//			lsmx.push_back(g);
//		}else
			ls.push_back(g);
		if(!g->grid_that_updates_this_glob)
			g->grid_that_updates_this_glob=this;
		return true;
	}
	bool splitif(const int nrec){
//		if((ls.size())<splitthresh)//? alg
		if((ls.size()+lsmx.size())<splitthresh)//? alg
			return false;
		if(nrec==0)
			return false;
		const float ns=s/2;
		grds[0]=new grid(ns,po+p3(-ns,ns,-ns));//?
		grds[1]=new grid(ns,po+p3( ns,ns,-ns));
		grds[2]=new grid(ns,po+p3(-ns,ns, ns));
		grds[3]=new grid(ns,po+p3( ns,ns, ns));

		grds[4]=new grid(ns,po+p3(-ns,-ns,-ns));
		grds[5]=new grid(ns,po+p3( ns,-ns,-ns));
		grds[6]=new grid(ns,po+p3(-ns,-ns, ns));
		grds[7]=new grid(ns,po+p3( ns,-ns, ns));

		for(auto o:ls)
			o->grid_that_updates_this_glob=nullptr;
		for(auto o:lsmx)
			o->grid_that_updates_this_glob=nullptr;

		for(auto g:grds){
			for(auto o:ls){
				g->putif(o,o->phy.p,o->phy.r);//?
			}
			for(auto o:lsmx){
				g->putif(o,o->phy.p,o->phy.r);//?
			}
			g->splitif(nrec-1);//?
		}
		ls.clear();
		lsmx.clear();
		return true;
	}
};
//size_t globs_per_grid_threshold=32;
//class grid{
//	vector<glob*>globs;
//	grid*subgrids[4];// 0:top left   1:top right   2:bottom left   3:bottom right
//public:
//	grid(){
//		p("new grid %p\n",this);
//		metrics::ngrid++;
//	}
//	~grid(){
//		p("delete grid %p\n",(void*)this);
//		foreach(globs,[this](glob*g){
//			if(g->grid_that_updates_this_glob==this)
//				delete g;
//		});
//		for(int i=0;i<4;i++){
//			delete subgrids[i];
//		}
//		metrics::ngrid--;
//	}
//	void add_glob(glob*g){
//		add_glob_rec(g,rec{{0,0,0},1});
//	}
//public:
//	void update_globs(){//! multicore
//		foreach(globs,[this](glob*g){
//			if(g->grid_that_updates_this_glob!=this)return;
//			g->update();
//		});
//	}
//	void render_globs(){foreach(globs,[](glob*g){g->render();});}// single thread opengl rendering
////	void remove_all_globs(){globs.clear();}
//	void render_grid_outline(){_render_outline(rec{p3{0,0,0},1});}
//private:
//	class rec{
//	public:
//		p3 pos;
//		floato scale;
//	};
//	void _render_outline(rec gr){
//		m4 m;
//		m.load_translate(gr.pos);
//		m.append_scaling(p3{gr.scale,gr.scale,gr.scale});
//		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,m.array());
//		glo_grid::instance.render();
//	}
//	void add_glob_rec(glob*g,rec rc){
//		globs.push_back(g);
//		if(globs.size()<globs_per_grid_threshold){
//			g->grid_that_updates_this_glob=this;
//			return;
//		}
//		// split
//		grid*sg[]=subgrids;
//		for_each(globs.begin(),globs.end(),[&rc,sg](glob*g){
//			const floato s=rc.scale/2;//scale of split grid
//			// put in top left?
////			const p3 tl=rc.pos-p3(-s,s,0);//top left center
//			const p3&gp=g->phy.p;//ref to glob position
//			const floato r=g->phy.r;//ref to glob radius
//			g->grid_that_updates_this_glob=nullptr;
//			if( gp.x-r<rc.pos.x && gp.x+r>=rc.pos.x-s && gp.y-r>rc.pos.y && gp.y+r<=rc.pos.y-s ){
//				// in top left
//				if(!subgrids[0])
//					subgrids[0]=new grid();
//				subgrids[0]->add_glob_rec(g,rec{rc.pos-p3(-s,s,0),s});
//			}
//		});
//	}
//};
//------------------------------------------------------------------------------
#include<list>
#include<cstdlib>
class grid;// forward declaration
namespace gleso{
	inline floato d(const floato unit_over_second){return unit_over_second*dt;}
	unsigned int tick;//?? rollover issues when used in comparisons
	vector<shader*>shaders;
	vector<glo*>glos;
	vector<texture*>textures;
	grid grd(1);
	floato rnd(){return floato(rand())/RAND_MAX;}
	floato rnd(floato min,floato max){return min+(floato(rand())/RAND_MAX)*(max-min);}
}
//------------------------------------------------------------------------------
#include<sys/time.h>
namespace fps{
	float fps;
	struct timeval t0;
	static int frameno;
	static int last_frameno;
	static inline void reset(){
		gettimeofday(&t0,NULL);
	}
	static inline float dt(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		const time_t diff_s=tv.tv_sec-t0.tv_sec;
		const int diff_us=tv.tv_usec-t0.tv_usec;
		return (float)diff_s+diff_us/1000000.f;
	}
	inline void before_render(){
		frameno++;
	}
	void after_render(){
		const float d=dt();
		if(d<3)
			return;
		const int dframe=frameno-last_frameno;
		last_frameno=frameno;
		fps=dframe/d;
		metrics::fps=(unsigned int)fps;
		reset();
		metrics::log();
	}
}
//------------------------------------------------------------------------------
/*
   /////  ///\   ///// ///// /////////
   ///   ///\\\  ///   ///  // /// //
  ///   ///  \\\///   ///     ///
///// /////   \\\/  /////   /////
*/
class glo_ball:public glo{
	int nvertices;
public:
	glo_ball():nvertices(1+12+1){}
protected:
	virtual vector<GLfloat>make_vertices()const{
		vector<GLfloat>v;
		v.push_back(0);//x
		v.push_back(0);//y
		floato rad=0;
		const floato drad=2*float(M_PI/12);
		for(int i=1;i<=nvertices;i++){
			const floato x=cosf(rad);
			const floato y=sinf(rad);
			rad+=drad;
			v.push_back(x);
			v.push_back(y);
		}
		return v;
	}
	virtual vector<GLfloat>make_colors()const{
		vector<GLfloat>v;
		const int n=nvertices*4;
		for(int k=0;k<n;k++){
			v.push_back(1);
		}
		return v;
	}
	virtual void gldraw()const{
		glDrawArrays(GL_TRIANGLE_FAN,0,nvertices);
	}
public:
	static glo_ball instance;
};
glo_ball glo_ball::instance=glo_ball();

class a_ball:public glob{
public:
	a_ball(){
		gl=&glo_ball::instance;
		phy.p.x=gleso::rnd(-1,1);
		phy.p.y=gleso::rnd(-1,1);
		phy.r=.0025;
		phy.s=p3{phy.r,phy.r,phy.r};
		phy.dp.x=gleso::rnd(-1,1);
		phy.dp.y=gleso::rnd(-1,1);
//		phy.p.z=-1;
	}
	virtual void on_update(){
//		p("update desk  %f    \n",phy.p.x);
		const floato d=.1;
		if(phy.p.x>1-phy.s.x)
			phy.dp.x=-d;
		else if(phy.p.x<-1+phy.s.x)
			phy.dp.x=d;
		if(phy.p.y>1-phy.s.y)
			phy.dp.y=-d;
		else if(phy.p.y<-1+phy.s.y)
			phy.dp.y=d;
	}
};

static list<glob*>all_globs;
static void gleso_impl_setup(){
	gleso::shaders.push_back(&shader::instance);
	gleso::textures.push_back(&texture::instance);//?? leak
	gleso::glos.push_back(&glo::instance);
	gleso::glos.push_back(&glo_grid::instance);
	gleso::glos.push_back(&glo_ball::instance);
	const int instances=1024*2;
	for(int n=0;n<instances;n++)
		all_globs.push_back(new a_ball());
}
/*
 ascii sprite kit

             ____
            |O  O|
            |_  _|         <- "medusa"
             /||\

 _______________
   |____|     <- stuff to slide under
  //||\|\\


 _______________     <- floor and roof




     _\|/_
 _____/ \_______   <- stuff to jump over



       stuff that scores -->   . o O *     <-- stuff to avoid

*/

//////
////
//  interface
#include<signal.h>
//#include<execinfo.h>
static void mainsig(const int i){
	p(" ••• terminated with signal %d\n",i);
//	const int nva=10;
//	void*va[nva];
//	int n=backtrace(va,nva);
//	backtrace_symbols_fd(va,n,1);
	exit(i);
}
#include<typeinfo>
static struct timeval timeval_after_init;
static camera*c;
int gleso_init(){
	p("* gleso\n");
	for(int i=0;i<32;i++)signal(i,mainsig);//?
	shader::check_gl_error("init");
	shader::print_gl_string("GL_VERSION",GL_VERSION);
	shader::print_gl_string("GL_VENDOR",GL_VENDOR);
	shader::print_gl_string("GL_RENDERER",GL_RENDERER);
	//	    printGLString("Extensions",GL_EXTENSIONS);
	shader::print_gl_string("GL_SHADING_LANGUAGE_VERSION",GL_SHADING_LANGUAGE_VERSION);
	shader::check_gl_error("after opengl info");
	p("* types\n");
	p("%16s %4u B\n","int",(unsigned int)sizeof(int));
	p("%16s %4u B\n","float",(unsigned int)sizeof(float));
	p("%16s %4u B\n","long",(unsigned int)sizeof(long));
	p("%16s %4u B\n","long long",(unsigned int)sizeof(long long));
	p("%16s %4u B\n","double",(unsigned int)sizeof(double));
	p("%16s %4u B\n","long double",(unsigned int)sizeof(long double));
	p("%16s %4u B\n","short",(unsigned int)sizeof(short));
	p("%16s %4u B\n","bool",(unsigned int)sizeof(bool));
	p("%16s %4u B\n","char",(unsigned int)sizeof(char));
	p("%16s %4u B\n","p3",(unsigned int)sizeof(p3));
	p("%16s %4u B\n","m4",(unsigned int)sizeof(m4));
	p("%16s %4u B\n","glo",(unsigned int)sizeof(glo));
	p("%16s %4u B\n","glob",(unsigned int)sizeof(glob));
	p("%16s %4u B\n","grid",(unsigned int)sizeof(grid));
	p("%16s %4u B\n","physics",(unsigned int)sizeof(physics));
	srand(1);// generate same random numbers in different instances
	if(!gl::shdr){// init
		p("* init\n");
		gl::shdr=&shader::instance;
		c=new camera();
		all_globs.push_back(c);
		gleso_impl_setup();
	}
	gl::active_program=0;
	p("* load\n");
	foreach(gleso::shaders,[](shader*o){
		p(" shader %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	foreach(gleso::textures,[](texture*o){
		p(" texture %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	foreach(gleso::glos,[](glo*o){
		p(" glo %p   %s\n",(void*)o,typeid(*o).name());
		o->load();
	});
	fps::reset();
	gettimeofday(&timeval_after_init,NULL);
	metrics::time_since_start_in_seconds=0;
	return 0;
}
void gleso_viewport(int width,int height){
	p("* viewport  %d x %d\n",width,height);
	if(gl::shdr)gl::shdr->viewport(width,height);
	c->screen_width=width;
	c->screen_height=height;
}

static bool render_globs=true;
static bool render_grid_outline=true;
void gleso_step(){
	fps::before_render();
	metrics::before_render();
	gleso::tick++;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	const time_t diff_s=tv.tv_sec-timeval_after_init.tv_sec;
	const int diff_us=tv.tv_usec-timeval_after_init.tv_usec;
	metrics::time_since_start_in_seconds=(float)diff_s+diff_us/1000000.f;
	gleso::dt=floato(1./60);

//	p("*** gleso tick:%d\n",gleso::tick);
	gleso::grd.clear();
	gleso::grd.addall(all_globs);
	gleso::grd.update_globs();//? thread

	c->pre_render();
	if(render_globs)gleso::grd.render_globs();//? thread
	if(render_grid_outline)gleso::grd.render_outline();
	fps::after_render();
//	metrics::log();
}
//void gleso_on_context_destroyed(){
//	if(gl::shdr)delete gl::shdr;
//	std::for_each(gleso::glos.begin(),gleso::glos.end(),[](glo*g){delete g;});
//	if(gleso::grd)delete gleso::grd;
//}
///////////////////////////////
void gleso_key(int key,int scancode,int action,int mods){
	p("gleso_key  key=%d   scancode=%d    action=%d   mods=%d\n",key,scancode,action,mods);
	switch(key){
	case 87://w
		switch(action){
			case 1:c->phy.dp.y=1;break;
			case 0:c->phy.dp.y=0;break;
		}
		break;
	case 83://s
		switch(action){
			case 1:c->phy.dp.y=-1;break;
			case 0:c->phy.dp.y=0;break;
		}
		break;
	case 65://a
		switch(action){
//			case 1:c->phy.dp.x=-1;break;
//			case 0:c->phy.dp.x=0;break;
			case 1:c->phy.da.z=180;break;
			case 0:c->phy.da.z=0;break;
		}
		break;
	case 68://d
		switch(action){
//			case 1:c->phy.dp.x=1;break;
//			case 0:c->phy.dp.x=0;break;
			case 1:c->phy.da.z=-180;break;
			case 0:c->phy.da.z=0;break;
		}
		break;
	}
}
void gleso_touch(floato x,floato y,int action){
	p("gleso_touch  x=%.1f   y=%.1f    action=%d\n",x,y,action);
}
void gleso_cleanup(){
//	for(auto o:all_globs){
//		delete o;
//	}
	p(" shaders:%d   grids:%d   glos:%d   globs:%d\n",metrics::nshader,metrics::ngrid,metrics::nglo,metrics::nglob);
}
