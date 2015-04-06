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
	GLuint umvp;// mat4 model-world-view-projection matrix
	GLuint utex;// texture sampler
}
////////////////////////////////////////////////////////////////////////
#include<string>
class shader{
	GLuint glid_program{0};
	GLuint apos{0};
	GLuint auv{0};
	GLuint umvp{0};
	GLuint utex{0};
public:
	shader(){metrics::nshader++;}

	virtual~shader(){
		metrics::nshader--;
		if(glid_program){glDeleteProgram(glid_program);glid_program=0;}
	}

	static void printGLString(const char *name,const GLenum s){
		const char*v=(const char*)glGetString(s);
		p("GL %s = %s\n",name,v);
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

	static bool checkGlError(const char*op){
		bool err=false;
		for(GLenum error=glGetError();error;error=glGetError()){
			p("at %s() glError (0x%x):  %s\n",op,error,get_gl_error_string(error));
			err=true;
		}
		return err;
	}

	static GLuint loadShader(const GLenum shader_type,const char*source){
		//throw "error";
 		const GLuint shader=glCreateShader(shader_type);
 		if(!shader)return 0;
		glShaderSource(shader,1,&source,NULL);
		glCompileShader(shader);
		GLint compiled=0;
		glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
//		LOGE("compiled: %d\n",compiled);
		if(compiled)return shader;
		GLint infolen=0;
		glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&infolen);
//		LOGE("info log len: %d\n",infolen);
		if(!infolen)return 0;
		char*buf=(char*)malloc(size_t(infolen));
		if(!buf)return 0;
		glGetShaderInfoLog(shader,infolen,NULL,buf);
		p("Could not compile shader %d:\n%s\n",shader_type, buf);
		free(buf);
		glDeleteShader(shader);
		return 0;
	}

	bool load(){
		createProgram(vertex_shader_source(),fragment_shader_source());
		if(checkGlError("program"))return false;
		return !bind();
	}

	void viewport(const int wi,const int hi){
		glViewport(0,0,wi,hi);
	}

	void use_program(){
		glUseProgram(glid_program);
		prepare_gl_for_render();
	}

private:
	bool createProgram(const char*vertex_shader_source,const char*fragment_shader_source){
		GLuint glid_vertex_shader=loadShader(GL_VERTEX_SHADER,vertex_shader_source);
		if(!glid_vertex_shader)return false;
		GLuint glid_pixel_shader=loadShader(GL_FRAGMENT_SHADER,fragment_shader_source);
		if(!glid_pixel_shader)return false;
		glid_program=glCreateProgram();
		if(!glid_program)return false;
		glAttachShader(glid_program,glid_vertex_shader);
		if(checkGlError("glAttachShader vertex"))return false;
		glAttachShader(glid_program,glid_pixel_shader);
		if(checkGlError("glAttachShader fragment"))return false;
		glLinkProgram(glid_program);
		GLint linkStatus=GL_FALSE;
		glGetProgramiv(glid_program,GL_LINK_STATUS,&linkStatus);
		if(linkStatus)return true;
		GLint bufLength=0;
		glGetProgramiv(glid_program,GL_INFO_LOG_LENGTH,&bufLength);
		if(bufLength){
			char*buf=(char*)malloc(size_t(bufLength));
			if(buf){
				glGetProgramInfoLog(glid_program,bufLength,NULL,buf);
				p("Could not link program:\n%s\n",buf);
				free(buf);
			}
		}
		glDeleteProgram(glid_program);
		glid_program=0;
		return false;
	}
protected:
	inline GLint get_attribute_location(const char*name){return glGetAttribLocation(glid_program,name);}
	inline GLint get_uniform_location(const char*name){return glGetUniformLocation(glid_program,name);}

const char*shader_source_vertex=R"(
#version 100
uniform mat4 umvp;
attribute vec4 apos;
attribute vec2 auv;
varying vec2 vuv;
void main(){
	gl_Position=umvp*apos;
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

	#define A(x,y)if((x=(GLuint)get_attribute_location(y))==(GLuint)-1){p("shader: cannot find attribute %s\n",y);return-1;};
	#define U(x,y)if((x=(GLuint)get_uniform_location(y))==(GLuint)-1){p("shader: cannot find uniform %s\n",y);return-1;}
	virtual int bind(){
		A(apos,"apos");
		A(auv,"auv");
		U(umvp,"umvp");
		U(utex,"utex");
		return 0;
	}
	inline virtual void prepare_gl_for_render(){
		gl::umvp=umvp;
		gl::utex=utex;
		gl::apos=apos;
		gl::auv=auv;
	}
};

////////////////////////////////////////////////
#include<vector>
using std::vector;

const int texture_width=2,texture_height=2;
GLubyte texels_rgb[]={
	0xff,0x00,0x00,   0x00,0xff,0x00,    0x00, 0x00,
	0x00,0x00,0xff,   0xff,0xff,0x00,    0x00, 0x00,
};
class texture{
public:
	void load(){
		glGenTextures(1,&glid_texture);
		glBindTexture(GL_TEXTURE_2D,glid_texture);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,texture_width,texture_height,0,GL_RGB,GL_UNSIGNED_BYTE,(GLvoid*)texels_rgb);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	void enable_for_gl_draw(){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,glid_texture);
		glUniform1i(glid_texture,0);
	}
	void refresh_from_data(){
		glBindTexture(GL_TEXTURE_2D,glid_texture);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,texture_width,texture_height,0,GL_RGB,GL_UNSIGNED_BYTE,texels_rgb);
	}
private:
	GLuint glid_texture{0};
};
class glo{
public:
#ifdef GLESO_EMBEDDED
	vector<GLfloat>vertices;
	vector<GLfloat>texture_coords;
	class texture*tex{nullptr};
#else
	GLuint glid_vao{0};
	GLuint glid_buffer_vertices{0};
#endif
	glo(){metrics::nglo++;}
	virtual~glo(){metrics::nglo--;}
	inline glo&texture(texture*t){tex=t;return*this;}
	int load(){// called when context is (re)created
#ifdef GLESO_EMBEDDED
		vertices=make_vertices();
		texture_coords=make_texture_coords();
#else
		glGenVertexArrays(1,&glid_vao);
		glBindVertexArray(glid_vao);
		glGenBuffers(1,&glid_buffer_vertices);
		glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
		const vector<GLfloat>vec=make_vertices();
		glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(vec.size()*sizeof(GLfloat)),vec.data(),GL_STATIC_DRAW);
		if(shader::checkGlError("load"))return 1;
#endif
		return 0;
	}
	void render()const{
#ifdef GLESO_EMBEDDED
		glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,&vertices[0]);
#else
		glBindVertexArray(glid_vao);
		glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
		glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,0);
#endif
		glEnableVertexAttribArray(gl::apos);

		if(tex){
			glVertexAttribPointer(gl::auv,2,GL_FLOAT,GL_FALSE,0,&texture_coords[0]);
			glEnableVertexAttribArray(gl::auv);
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
	inline p3():x_(0),y_(0),z_(0){}
	inline p3(const floato x):x_(x),y_(0),z_(0){}
	inline p3(const floato x,const floato y):x_(x),y_(y),z_(0){}
	inline p3(const p3&p):x_(p.x_),y_(p.y_),z_(p.z_){}
	inline p3&x(const floato f){x_=f;return*this;}inline floato x()const{return x_;}
	inline p3&y(const floato f){y_=f;return*this;}inline floato y()const{return y_;}
	inline p3&z(const floato f){z_=f;return*this;}inline floato z()const{return z_;}
	inline p3&add(const p3&p,const floato dt){x_+=p.x_*dt;y_+=p.y_*dt;z_+=p.z_*dt;return*this;}//? simd
private:
	floato x_,y_,z_;
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
	inline p3&position(){return p;}
	inline p3&pos(){return p;}
	inline p3&dpos(){return dp;}
	inline p3&angle(){return a;}
	inline p3&dagl(){return da;}
	inline p3&scale(){return s;}
private:
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
// lifted from apple examples
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
class m4{
	floato c[16]{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
public:
	inline m4&load_translate(const p3&p){mtxLoadTranslate(c,p.x(),p.y(),p.z());return*this;}
	inline m4&append_rotation_about_z_axis(const floato degrees){mtxRotateZApply(c,degrees);return*this;}
	inline m4&append_scaling(const p3&scale){mtxScaleApply(c,scale.x(),scale.y(),scale.z());return*this;}
	inline const floato*array()const{return c;}
};
//class linked_list{
//	linked_list*nxt;
//	linked_list*prv;
//};
//class glob:public linked_list{
//glo nullglo{};
class glob{
	const class glo*glo{nullptr};// ref to gl renderable
	class physics phys;// current physics state
	class physics phys_prv;// previous physics state
	class physics phys_nxt;// next physics state, computed during update
	m4 matrix_model_world;
	class render_info render_info;// info for opengl rendering
	class render_info render_info_next;// next renderinfo, updated during render
	p3 scal;
public:
	glob(){metrics::nglob++;}
	virtual ~glob(){}
	inline glob&glo_ref(const class glo*g){glo=g;return*this;}
	inline class physics&physics(){return phys;}
	inline const p3&scale()const{return scal;}
	inline glob&scale(const p3&scale){scal=scale;return*this;}
	void render(){
		if(!glo)return;
		render_info=render_info_next;
		matrix_model_world.load_translate(render_info.position());
		matrix_model_world.append_rotation_about_z_axis(render_info.angle().z());
		matrix_model_world.append_scaling(render_info.scale());
		glUniformMatrix4fv(GLint(gl::umvp),1,false,matrix_model_world.array());
		glo->render();
	}
	void update(){
		phys_prv=phys;
		phys.scale()=scal;
		phys.update();
		on_update();
		render_info_next.position(phys.position());
		render_info_next.angle(phys.angle());
		render_info_next.scale(phys.scale());
	}
	virtual void on_update(){
		if(phys.pos().x()>1)
			phys.dpos().x(-phys.dpos().x());
		else if(phys.pos().x()<-1)
			phys.dpos().x(-phys.dpos().x());
	}
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
int gleso_init(){
	shader::checkGlError("init");
	shader::printGLString("GL_VERSION",GL_VERSION);
	shader::printGLString("GL_VENDOR",GL_VENDOR);
	shader::printGLString("GL_RENDERER",GL_RENDERER);
	//	    printGLString("Extensions",GL_EXTENSIONS);
	shader::printGLString("GL_SHADING_LANGUAGE_VERSION",GL_SHADING_LANGUAGE_VERSION);
	shader::checkGlError("after opengl info");

	p("/// gleso init\n");
	p("%16s %4u B\n","int",(unsigned int)sizeof(int));
	p("%16s %4u B\n","float",(unsigned int)sizeof(floato));
	p("%16s %4u B\n","p3",(unsigned int)sizeof(p3));
	p("%16s %4u B\n","m4",(unsigned int)sizeof(m4));
	p("%16s %4u B\n","glo",(unsigned int)sizeof(glo));
	p("%16s %4u B\n","glob",(unsigned int)sizeof(glob));
	p("%16s %4u B\n","grid",(unsigned int)sizeof(grid));
	p("%16s %4lu B\n","physics",sizeof(physics));
	srand(1);// generate same random numbers in different instances
	p("\n");

	if(!gl::shdr)gl::shdr=new shader();
	if(!gl::shdr->load())return 1;

	if(gleso::glos.empty()){//? if no glos declared re-init?
		gleso_impl_add_resources();
		foreach(gleso::textures,[](texture*o){
			p(" texture %p   %s\n",(void*)o,typeid(*o).name());
			o->load();
		});
		foreach(gleso::glos,[](glo*o){
			p(" glo %p   %s\n",(void*)o,typeid(*o).name());
			o->load();
		});
	}
	if(!gleso::grd){
		gleso::grd=new grid();
		gleso::grd->add(/*gives*/gleso_impl_create_root());//? leak? grd->add does not take
	}
	fps::reset();
	gettimeofday(&timeval_after_init,NULL);
	metrics::time_since_start_in_seconds=0;
	return 0;
}
void gleso_viewport(int width,int height){
	p("/// gleso_viewport  %d x %d\n",width,height);
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
	glClearColor(floato{.5},0,floato{.5},1);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	gl::shdr->use_program();
	gleso::grd->update();//? thread
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
