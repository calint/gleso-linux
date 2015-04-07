#include "gleso.h"
////////////////////////////////////////////////////////////////////////
namespace metrics{
	unsigned int fps;
	unsigned int nshader;
	unsigned int nglo;
	unsigned int nglob;
	float time_since_start_in_seconds;
	void log(){p("/ metrics %2.2fs – fps:%03d – shaders:%01d – glos:%02d – globs:%05d\n",time_since_start_in_seconds,fps,nshader,nglo,nglob);}
}
////////////////////////////////////////////////////////////////////////
class shader;
namespace gl{
	shader*shdr;
	GLuint apos;// vec2 vertex coords x,y
	GLuint auv;// vec2 texture coords x,y
	GLuint umtx_mw;// mat4 model-world matrix
	GLuint umtx_vp;// mat4 view projection matrix
	GLuint utex;// texture sampler
}
////////////////////////////////////////////////////////////////////////
#include<string>
class shader{
	GLuint glid_program{0};
	GLuint apos{0};
	GLuint auv{0};
	GLuint umtx_mw{0};
	GLuint umtx_vp{0};
	GLuint utex{0};
public:
	shader(){metrics::nshader++;}

	virtual~shader(){
		metrics::nshader--;
		p("deleting shader %p\n",(void*)this);
//		if(glid_program){glDeleteProgram(glid_program);glid_program=0;}
	}

	static void print_gl_enum(const char *name,const GLenum s){
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
		for(GLenum error=glGetError();error;error=glGetError()){
			p("!!! %s   glerror %x   %s\n",op,error,get_gl_error_string(error));
			throw"detected gl error";
		}
	}
	static const char*get_shader_name_for_type(GLenum shader_type){
		switch(shader_type){
		case GL_VERTEX_SHADER:return"vertex";
		case GL_FRAGMENT_SHADER:return"fragment";
		default:return"unknown";
		}
	}
	static GLuint loadShader(const GLenum shader_type,const char*source){
		//throw "error";
 		const GLuint shader=glCreateShader(shader_type);
 		p(" %s shader glid=%d\n",get_shader_name_for_type(shader_type),shader);
 		if(!shader)throw"cannot get shader id";
 		glShaderSource(shader,1,&source,NULL);
		glCompileShader(shader);
		GLint compiled=0;
		glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
		if(compiled)return shader;
		GLint infolen=0;
		glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&infolen);
		if(!infolen)throw"cannot get infolen";
		char*buf=new char[infolen];//(char*)malloc(size_t(infolen));
		if(!buf)throw"cannot get compiler error";
		glGetShaderInfoLog(shader,infolen,NULL,buf);
		p("!!! could not compile %s shader:\n%s\n",get_shader_name_for_type(shader_type),buf);
		free(buf);
		glDeleteShader(shader);
		throw"could not compile shader";
	}

	void load(){
		createProgram(vertex_shader_source(),fragment_shader_source());
		check_gl_error("program");
		bind();
	}

	void viewport(const int wi,const int hi){
		glViewport(0,0,wi,hi);
	}

	void use_program(){
		glUseProgram(glid_program);
		prepare_gl_for_render();
	}

private:
	void createProgram(const char*vertex_shader_source,const char*fragment_shader_source){
		GLuint glid_vertex_shader=loadShader(GL_VERTEX_SHADER,vertex_shader_source);
		GLuint glid_pixel_shader=loadShader(GL_FRAGMENT_SHADER,fragment_shader_source);
		glid_program=glCreateProgram();
		if(!glid_program)throw"cannot create program";
		p(" program glid=%d\n",glid_program);
		glAttachShader(glid_program,glid_vertex_shader);
		check_gl_error("glAttachShader vertex");
		glAttachShader(glid_program,glid_pixel_shader);
		check_gl_error("glAttachShader fragment");
		glLinkProgram(glid_program);
		GLint linkStatus=GL_FALSE;
		glGetProgramiv(glid_program,GL_LINK_STATUS,&linkStatus);
		if(linkStatus)return;
		GLint bufLength=0;
		glGetProgramiv(glid_program,GL_INFO_LOG_LENGTH,&bufLength);
		if(bufLength){
			char*buf=(char*)malloc(size_t(bufLength));
			if(buf){
				glGetProgramInfoLog(glid_program,bufLength,NULL,buf);
				p("!!! could not link program:\n%s\n",buf);
				free(buf);
			}
		}
		glDeleteProgram(glid_program);
		glid_program=0;
		throw"error while linking";
	}
protected:
	inline GLint get_attribute_location(const char*name){return glGetAttribLocation(glid_program,name);}
	inline GLint get_uniform_location(const char*name){return glGetUniformLocation(glid_program,name);}

const char*shader_source_vertex=R"(
#version 100
uniform mat4 umtx_mw;// model-world matrix
uniform mat4 umtx_vp;// view-projection matrix
attribute vec4 apos;// vertices
attribute vec2 auv;// texture coords
varying vec2 vuv;
void main(){
	gl_Position=umtx_vp*umtx_mw*apos;
    vuv=auv;
}
)";
const char*shader_source_fragment=R"(
#version 100
uniform sampler2D utex;
varying mediump vec2 vuv;
void main(){
	gl_FragColor=texture2D(utex,vuv);
}
)";
	inline virtual const char*vertex_shader_source()const{return shader_source_vertex;}
	inline virtual const char*fragment_shader_source()const{return shader_source_fragment;}

	#define A(x,y)if((x=(GLuint)get_attribute_location(y))==(GLuint)-1){p("shader: cannot find attribute %s\n",y);throw"error";};
	#define U(x,y)if((x=(GLuint)get_uniform_location(y))==(GLuint)-1){p("shader: cannot find uniform %s\n",y);throw"error";}
	virtual void bind(){
		A(apos,"apos");
		A(auv,"auv");
		U(umtx_mw,"umtx_mw");
		U(umtx_vp,"umtx_vp");
		U(utex,"utex");
	}
	inline virtual void prepare_gl_for_render(){
		gl::umtx_mw=umtx_mw;
		gl::umtx_vp=umtx_vp;
		gl::utex=utex;
		gl::apos=apos;
		gl::auv=auv;
	}
};

////////////////////////////////////////////////
#include<vector>
using std::vector;

class texture{
public:
//	~texture(){
//		p("deleting texture %p\n",this);
//		glDeleteTextures(1,&glid_texture);
//	}
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
		glBindTexture(GL_TEXTURE_2D,glid_texture);
		glUniform1i(glid_texture,0);
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
};
class glo{
	class texture*tex{nullptr};
#ifdef GLESO_EMBEDDED
	vector<GLfloat>vertices;
	vector<GLfloat>texture_coords;
#else
	GLuint glid_buffer_vertices{0};
	GLuint glid_buffer_texture_coords{0};
#endif
public:
	glo(){
		metrics::nglo++;
	}
	virtual~glo(){
		metrics::nglo--;
		p("deleting glo %p\n",this);
	}
	inline glo&texture(texture*t){tex=t;return*this;}
	int load(){// called when context is (re)created
#ifdef GLESO_EMBEDDED
		vertices=make_vertices();
		texture_coords=make_texture_coords();
#else
		glGenBuffers(1,&glid_buffer_vertices);
		p("    vertices buffer glid=%d\n",glid_buffer_vertices);
		glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
		const vector<GLfloat>v1=make_vertices();
		glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v1.size()*sizeof(GLfloat)),v1.data(),GL_STATIC_DRAW);
		shader::check_gl_error("load vertices");

		glGenBuffers(1,&glid_buffer_texture_coords);
		p("    texture coords buffer glid=%d\n",glid_buffer_texture_coords);
		glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_texture_coords);
		const vector<GLfloat>v2=make_texture_coords();
		glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v1.size()*sizeof(GLfloat)),v2.data(),GL_STATIC_DRAW);
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
			glVertexAttribPointer(gl::auv,2,GL_FLOAT,GL_FALSE,0,nullptr);
#endif
			tex->enable_for_gl_draw();
		}

		gldraw();

		glDisableVertexAttribArray(gl::apos);
		if(tex){
			glDisableVertexAttribArray(gl::auv);
		}
	}
protected:
	virtual vector<GLfloat>make_vertices()const{
		const GLfloat verts[]{0,1, -1,-1, 1,-1};
		vector<GLfloat>v;
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		const GLfloat verts[]{0,1, -1,-1, 1,-1};
		vector<GLfloat>v;
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual void gldraw()const{
		glDrawArrays(GL_TRIANGLES,0,3);
	}
};

class p3{
public:
	inline p3():x{0},y{0},z{0}{}
	inline p3(const floato x):x{x},y{0},z{0}{}
	inline p3(const floato x,const floato y):x{x},y{y},z{0}{}
//	inline p3(const p3&p):x{p.x},y{p.y},z{p.z}{}
//	inline p3&x(const floato f){x_=f;return*this;}inline floato x()const{return x_;}
//	inline p3&y(const floato f){y_=f;return*this;}inline floato y()const{return y_;}
//	inline p3&z(const floato f){z_=f;return*this;}inline floato z()const{return z_;}
	inline p3&add(const p3&p,const floato dt){x+=p.x*dt;y+=p.y*dt;z+=p.z*dt;return*this;}//? simd
//private:
	floato x,y,z;
};

#include<list>
#include<cstdlib>
class grid;// forward declaration
namespace gleso{
	floato dt;
	inline floato d(const floato unit_over_second){return unit_over_second*dt;}
	unsigned int tick;//?? rollover issues when used in comparisons
	vector<glo*>glos;
	vector<texture*>textures;
	grid*grd;
	floato rnd(){return floato(rand())/RAND_MAX;}
}

class physics{
public:
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
	p3 p,dp,ddp;//position
	p3 a,da,dda;//angle
	p3 s;//scale
	//? list of forces/velocities applied
};

class render_info{
public:
	inline const p3&position()const{return p;}
	inline render_info&position(const p3&pos){p=pos;return*this;}
	inline const p3&angle()const{return a;}
	inline render_info&angle(const p3&agl){a=agl;return*this;}
	inline const p3&scale()const{return s;}
	inline render_info&scale(const p3&scl){s=scl;return*this;}
private:
	p3 p,a,s;
};

////
// mtx funcs lifted from apple examples
static void mtxLoadTranslate(floato*mtx,const floato xTrans,const floato yTrans,const floato zTrans){
	// [ 0 4  8  x ]
	// [ 1 5  9  y ]
	// [ 2 6 10  z ]
	// [ 3 7 11 15 ]
	mtx[ 0] = mtx[ 5] = mtx[10] = mtx[15] = 1;
	
	mtx[ 1] = mtx[ 2] = mtx[ 3] = mtx[ 4] =
	mtx[ 6] = mtx[ 7] = mtx[ 8] = mtx[ 9] =
	mtx[11] = 0.0;
	
	mtx[12] = xTrans;
	mtx[13] = yTrans;
	mtx[14] = zTrans;
}
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
static void mtxScaleApply(floato* mtx, floato xScale, floato yScale, floato zScale)
{
	// [ 0 4  8 12 ]   [ x 0 0 0 ]
	// [ 1 5  9 13 ] x [ 0 y 0 0 ]
	// [ 2 6 10 14 ]   [ 0 0 z 0 ]
	// [ 3 7 11 15 ]   [ 0 0 0 1 ]
	
	mtx[ 0] *= xScale;
	mtx[ 4] *= yScale;
	mtx[ 8] *= zScale;
	
	mtx[ 1] *= xScale;
	mtx[ 5] *= yScale;
	mtx[ 9] *= zScale;
	
	mtx[ 2] *= xScale;
	mtx[ 6] *= yScale;
	mtx[10] *= zScale;
	
	mtx[ 3] *= xScale;
	mtx[ 7] *= yScale;
	mtx[11] *= xScale;
}
void mtxLoadOrthographic(float* mtx,
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
	mtx[ 5] = 2.0f / (top - bottom);
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
void mtxMultiply(float* ret, const float* lhs, const float* rhs)
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
	floato c[16]{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
public:
	inline m4&load_translate(const p3&p){mtxLoadTranslate(c,p.x,p.y,p.z);return*this;}
	inline m4&append_rotation_about_z_axis(const floato degrees){mtxRotateZApply(c,degrees);return*this;}
	inline m4&append_scaling(const p3&scale){mtxScaleApply(c,scale.x,scale.y,scale.z);return*this;}
	inline m4&load_ortho_projection(floato left,floato right,floato bottom,floato top,floato nearZ,floato farZ){
		mtxLoadOrthographic(c,left,right,bottom,top,nearZ,farZ);
		return*this;
	}
	inline const floato*array()const{return c;}
};

class glob{
	const class glo*glo{nullptr};// ref to gl renderable
	m4 matrix_model_world;
	class render_info render_info;// info for opengl rendering
	class render_info render_info_next;// next renderinfo, updated during render
	p3 scal;
protected:
	physics phy_prv;// previous physics state
//	physics phy_nxt;// next physics state
public:
	physics phy;// current physics state
	glob(){metrics::nglob++;}
	virtual ~glob(){}
	inline glob&glo_ref(const class glo*g){glo=g;return*this;}
//	inline physics&phys(){return phy;}
	inline const p3&scale()const{return scal;}
	inline glob&scale(const p3&scale){scal=scale;return*this;}
	void render(){
		if(!glo)return;
		render_info=render_info_next;
		matrix_model_world.load_translate(render_info.position());
		matrix_model_world.append_rotation_about_z_axis(render_info.angle().z);
		matrix_model_world.append_scaling(render_info.scale());
		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,matrix_model_world.array());
		glo->render();
	}
	void update(){
		phy_prv=phy;
//		phy=phy_nxt;
		phy.s=scal;
		phy.update();
		on_update();
		render_info_next.position(phy.p);
		render_info_next.angle(phy.a);
		render_info_next.scale(phy.s);
	}
	virtual void on_update(){}
};

class camera:public glob{
	m4 mtx_wp;
public:
	void pre_render(){
		gl::shdr->use_program();
		glClearColor(floato{.5},0,floato{.5},1);
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		mtx_wp.load_translate(phy.p);
		glUniformMatrix4fv(GLint(gl::umtx_vp),1,false,mtx_wp.array());
	}
	virtual void on_update(){
//		physics p=phys();
		p(" update camera  %f   %f\n",phy.p.x,phy.dp.x);
		if(phy.p.x>1)
			phy.dp.x={-1};
		else if(phy.p.x<-1)
			phy.dp.x={1};
		p(" update camera  %f   %f  after\n",phy.p.x,phy.dp.x);
	}
	inline const m4&matrix_world_view_projection()const{return mtx_wp;}
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
		const static GLfloat verts[]={-1,1, -1,-1, 1,-1, 1,1};
		vector<GLfloat>v;
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
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
		texture(gleso::textures[0]);
	}
	virtual vector<GLfloat>make_vertices()const{
		const static GLfloat verts[]{-1,1, -1,-1, 1,-1, 1,1};
		vector<GLfloat>v;
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		const static GLfloat verts[]{0,1, 0,0, 1,0, 1,1};
		vector<GLfloat>v;
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	inline virtual void gldraw()const{
//		texels_rgb[0]++;
//		texels_rgb[1]++;
		gleso::textures[0]->refresh_from_data();
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
	}
};
//------------------------------------------------------------------------------
#include<algorithm>
#define foreach(c,f)std::for_each(c.begin(),c.end(),f)
class grid{
	std::list<glob*>globs;//                        _
public://                                          (:)
	grid(){}//                                    __|__         <- "long neck"
	void add(glob*g){globs.push_back(g);}        //(.)\\          //
	void update(){foreach(globs,[](glob*g){g->update();});}//? multicore?
	void render(){foreach(globs,[](glob*g){g->render();});}// single thread opengl rendering
	void rem(glob*g){globs.remove(g);}//? multicore?||
	void clr(){globs.clear();}
	//    void refresh(){}// refreshes the grid, globs dont change grid often, globs often totally inside grid, maximum glob size less than grid    <-- procedurally generated text for vegetation
	~grid(){
		foreach(globs,[](glob*o){delete o;});
	}
};
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
static void gleso_impl_add_resources(){
	gleso::textures.push_back(/*gives*/new texture());//?? leak
	gleso::glos.push_back(/*gives*/new glo());//?? leak. push_pack does not /*take*/ ownership of glob
	gleso::glos.push_back(/*gives*/new glo_square_xy());//??
	gleso::glos.push_back(/*gives*/new glo_circle_xy());//??
	gleso::glos.push_back(/*gives*/new glo_square_xyuv());//??
}
static/*gives*/glob*gleso_impl_create_root(){
	glob*g=new glob();
	const floato s=floato{.9};
	g->scale(p3{s,s}).glo_ref(gleso::glos[3]);
	return g;
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
#include<typeinfo>
static struct timeval timeval_after_init;
static camera c;
int gleso_init(){
	p("* gleso\n");
	shader::check_gl_error("init");
	shader::print_gl_enum("GL_VERSION",GL_VERSION);
	shader::print_gl_enum("GL_VENDOR",GL_VENDOR);
	shader::print_gl_enum("GL_RENDERER",GL_RENDERER);
	//	    printGLString("Extensions",GL_EXTENSIONS);
	shader::print_gl_enum("GL_SHADING_LANGUAGE_VERSION",GL_SHADING_LANGUAGE_VERSION);
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
	p("%16s %4lu B\n","physics",sizeof(physics));
	srand(1);// generate same random numbers in different instances
	if(!gl::shdr){// init
		p("* init\n");
		gl::shdr=new shader();
		gleso_impl_add_resources();
		gleso::grd=new grid();
		gleso::grd->add(&c);
		gleso::grd->add(/*gives*/gleso_impl_create_root());//? leak? grd->add does not take
		c.phy.p.x={.5};
		c.phy.dp.x={1};
	}
	p("* load\n");
	gl::shdr->load();
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
}

void gleso_step(){
	fps::before_render();
	gleso::tick++;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	const time_t diff_s=tv.tv_sec-timeval_after_init.tv_sec;
	const int diff_us=tv.tv_usec-timeval_after_init.tv_usec;
	metrics::time_since_start_in_seconds=(float)diff_s+diff_us/1000000.f;
	gleso::dt=floato(1./60);
	gleso::grd->update();//? thread

//	p("%f  %f\n",cp.x(),gleso::dt);
	c.pre_render();
	gleso::grd->render();//? thread
	fps::after_render();
}
//void gleso_on_context_destroyed(){
//	if(gl::shdr)delete gl::shdr;
//	std::for_each(gleso::glos.begin(),gleso::glos.end(),[](glo*g){delete g;});
//	if(gleso::grd)delete gleso::grd;
//}
///////////////////////////////
void gleso_key(int key,int scancode,int action,int mods){
	p("gleso_key  key=%d   scancode=%d    action=%d   mods=%d\n",key,scancode,action,mods);
}
void gleso_touch(floato x,floato y,int action){
	p("gleso_touch  x=%.1f   y=%.1f    action=%d\n",x,y,action);
}
