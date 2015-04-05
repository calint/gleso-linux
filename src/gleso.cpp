#include "gleso.h"

#include<stdlib.h>
#include<string.h>
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
	GLuint apos;// vec4 position x,y,z,w
    GLuint umvp;// mat4 model-world-view-projection matrix
}
////////////////////////////////////////////////////////////////////////



class shader{
	GLuint glid_program;
	GLuint apos,umvp;
public:
    shader():glid_program(0),apos(0),umvp(0){metrics::nshader++;}
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
	void viewport(const int wi,const int hi){glViewport(0,0,wi,hi);}
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
    
#define shader_source_vertex "#version 100\nuniform mat4 umvp;attribute vec4 apos;void main(){gl_Position=umvp*apos;}"
#define shader_source_fragment "#version 100\nvoid main(){gl_FragColor=vec4(gl_FragCoord.x,gl_FragCoord.y,.2,1.);}"
    inline virtual const char*vertex_shader_source()const{return shader_source_vertex;}
    inline virtual const char*fragment_shader_source()const{return shader_source_fragment;}
    
    #define A(x,y)if((x=(GLuint)get_attribute_location(y))==(GLuint)-1)return-1;
    #define U(x,y)if((x=(GLuint)get_uniform_location(y))==(GLuint)-1)return-1;
    virtual int bind(){
        A(apos,"apos");
        U(umvp,"umvp");
        return 0;
    }
    inline virtual void prepare_gl_for_render(){
        gl::apos=apos;
        gl::umvp=umvp;
    }
};

////////////////////////////////////////////////
#include<vector>
class glo{
public:
#ifdef GLESO_EMBEDDED
    std::vector<GLfloat>vertices;
    glo(){metrics::nglo++;}
#else
    GLuint glid_vao;
    GLuint glid_buffer_vertices;
    glo():glid_vao(0),glid_buffer_vertices(0){metrics::nglo++;}
#endif
	virtual~glo(){metrics::nglo--;}
	int load(){
//		p("glo load\n");
#ifdef GLESO_EMBEDDED
        vertices=make_vertices();
#else
        glGenVertexArrays(1,&glid_vao);
        glBindVertexArray(glid_vao);
        glGenBuffers(1,&glid_buffer_vertices);
        glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
        const std::vector<GLfloat>vec=make_vertices();
        glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(vec.size()*sizeof(GLfloat)),vec.data(),GL_STATIC_DRAW);
        if(shader::checkGlError("load"))return 1;
#endif
		return 0;
	}
	void render()const{
#ifdef GLESO_EMBEDDED
//	    glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,vertices.data());
	    glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,&vertices[0]);
	    glEnableVertexAttribArray(gl::apos);
#else
        glBindVertexArray(glid_vao);
        glBindBuffer(GL_ARRAY_BUFFER,glid_buffer_vertices);
        glEnableVertexAttribArray(gl::apos);
        glVertexAttribPointer(gl::apos,2,GL_FLOAT,GL_FALSE,0,0);
#endif
        gldraw();
    }
protected:
    virtual std::vector<GLfloat>make_vertices()const{
//    	p(" load glo\n");
        const GLfloat verts[]={0,.5f, -.5f,-.5f, .5f,-.5f};
        std::vector<GLfloat>v;
        v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
        return v;
//
//        return std::vector<GLfloat>(std::begin(verts),std::end(verts));
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
    inline p3&x(const floato f){x_=f;return*this;}inline const floato x()const{return x_;}
    inline p3&y(const floato f){y_=f;return*this;}inline const floato y()const{return y_;}
    inline p3&z(const floato f){z_=f;return*this;}inline const floato z()const{return z_;}
    inline p3&add(const p3&p,const floato dt){x_+=p.x_*dt;y_+=p.y_*dt;z_+=p.z_*dt;return*this;}//? simd
private:
    floato x_,y_,z_;
};

#include<list>
class grid;
namespace gleso{
    floato dt;
    inline floato d(const floato unit_over_second){return unit_over_second*dt;}
    unsigned int tick;//?? rollover issues when used in comparisons
    std::vector<glo*>glos;
    grid*grd;
    const floato rnd(){return floato(rand())/RAND_MAX;}
}

class physics{
public:
    void update(){
        dp.add(ddp,gleso::dt);
        p.add(dp,gleso::dt);
        da.add(dda,gleso::dt);
        a.add(da,gleso::dt);
    }
    inline p3&pos(){return p;}
    inline p3&position(){return p;}inline p3&dpos(){return dp;}
    inline p3&angle(){return a;}inline p3&dagl(){return da;}
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
void mtxScaleApply(floato* mtx, floato xScale, floato yScale, floato zScale)
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
    floato c[16];
public:
    m4():c{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }{}
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
class glob{
	const class glo*glo;
    class physics phys;// current physics state
    class physics phys_prv;// previous physics state
    class physics phys_nxt;// next physics state, computed during update
    m4 matrix_model_world;
    class render_info render_info;// info for opengl rendering
    class render_info render_info_next;// next renderinfo, updated during render
    p3 scal;
public:
	glob():glo(0){metrics::nglob++;}
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
//        LOGI("scale x %f\n",render_info.scale().x());
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
        if(phys.pos().x()>1)phys.dpos().x(-phys.dpos().x());
        else if(phys.pos().x()<-1)phys.dpos().x(-phys.dpos().x());
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
    virtual std::vector<GLfloat>make_vertices()const{
        const static GLfloat verts[]={-1,1, -1,-1, 1,-1, 1,1};
        std::vector<GLfloat>v;
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
    virtual std::vector<GLfloat>make_vertices()const{
        std::vector<GLfloat>v;
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
//        p("circle  virtual call   vertices size %d\n",int(v.size()));
        return v;
    }
    virtual void gldraw()const{
        glDrawArrays(GL_TRIANGLE_FAN,0,nvertices);
    }
    
};


   /////  ///\   ///// ///// /////////
   ///   ///\\\  ///   ///  // /// //
  ///   ///  \\\///   ///     ///
///// /////   \\\/  /////   /////


//#ifndef GLESO_EMBEDDED
//const int nsprites=1024*16;
//#else
//const int nsprites=1024;
//#endif
//const int nsprites=32;//1024*4;
static void gleso_impl_add_glos(std::vector<glo*>&glos){
    glos.push_back(/*gives*/new glo());//? leak. push_pack does not /*take*/ ownership of glob
    glos.push_back(/*gives*/new glo_square_xy());//? leak. push_pack does not /*takes*/
    glos.push_back(/*gives*/new glo_circle_xy());//? leak. /*gives*/ not matched by /*takes*/
}
static/*gives*/glob*gleso_impl_create_root(){
    glob*g=new glob();
    const floato s=floato(.025);
    g->scale(p3{s,s}).glo_ref(gleso::glos[2]);
    physics&p=g->physics();
    p.pos().x(2*gleso::rnd()-1);
    p.pos().y(2*gleso::rnd()-1);
    p.dpos().x(floato(.01));
    p.dagl().z(360/60);
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

#include<sys/time.h>
//static void log_metrics();
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
        //        t0=tv;
        return (float)diff_s+diff_us/1000000.f;
    }
	inline void before_render(){
        frameno++;
    }
	void after_render(){
		const float d=dt();
        //        LOGI("dt: %f\n",d);
		if(d>3){
			const int dframe=frameno-last_frameno;
			last_frameno=frameno;
            fps=dframe/d;
            metrics::fps=(unsigned int)fps;
			reset();
            metrics::log();
		}
	}
}

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
//	p("%16s %4lu B\n","physics",sizeof(physics));
    srand(1);// generate same random numbers in different instances
    int nsprite=1024;
//    p("\nargs:\n");
//    char**a=gleso_argv;// command line arguments
//    int c=gleso_argc;
//    while(c--){
//        const char*s=*a++;
//        if(*s!='-')continue;// looking for example: -s 100
//        if(*(s+1)=='s'){// found -s
//            const char*arg=*a++;//
////            p("nsprite=%s\n",arg);
//            nsprite=atoi(arg);
////            p("nsprite=%d\n",nsprite);
//            break;
//        }
//    }
    p("\n\n");

	if(!gl::shdr)gl::shdr=new shader();
    if(!gl::shdr->load())return 1;
    
    if(gleso::glos.empty()){//? if no glos declared re-init?
        gleso_impl_add_glos(gleso::glos);
//        p(" glos %d\n",gleso::glos.size());
//        for(glo*g:gleso::glos){
//            p(" glo %p   %s\n",g,typeid(*g).name());
//        }
        foreach(gleso::glos,[](glo*g){
//            p(" glo %p   %s\n",g,typeid(*g).name());
        	g->load();
        });
    }
    if(!gleso::grd){
    	gleso::grd=new grid();
        for(int i=0;i<nsprite;i++)
            gleso::grd->add(/*gives*/gleso_impl_create_root());//? leak? grd->add does not take
    }
    fps::reset();
    gettimeofday(&timeval_after_init,NULL);
    metrics::time_since_start_in_seconds=0;
    return 0;
}
void gleso_on_viewport_change(int width,int height){
	p("/// gleso_on_viewport_change %d x %d\n",width,height);
	if(gl::shdr)gl::shdr->viewport(width,height);
}
static struct timeval tv;
void gleso_step(){
	fps::before_render();
    gleso::tick++;
    gettimeofday(&tv,NULL);
    const time_t diff_s=tv.tv_sec-timeval_after_init.tv_sec;
    const int diff_us=tv.tv_usec-timeval_after_init.tv_usec;
    metrics::time_since_start_in_seconds=(float)diff_s+diff_us/1000000.f;

    gleso::dt=floato(1./60);
    glClearColor(0,0,floato(.2),1);
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
	printf("gleso_key  key=%d   scancode=%d    action=%d   mods=%d\n",key,scancode,action,mods);
}
void gleso_touch(floato x,floato y,int action){
	printf("gleso_touch  x=%f   y=%f    action=%d\n",x,y,action);
}



//   clipboard


//static const char vertex_shader_source[]="attribute vec4 apos;void main(){gl_Position=apos;}";
//static const char fragment_shader_source[]="precision mediump float;void main(){gl_FragColor=vec4(0.0,1.0,0.0,1.0);}";
//static const char vertex_shader_source[]="#version 100\nuniform mat4 umvp;attribute vec4 apos;void main(){gl_Position=umvp*apos;}";
////static const char fragment_shader_source[]="#version 100\nvoid main(){gl_FragColor=vec4(.6,.3,.2,1.);}";
//static const char vertex_shader_source_ch[]="#version 100\nuniform mat4 umvp;attribute vec4 apos;void main(){gl_Position=umvp*apos;}";
//static const char fragment_shader_source_ch[]="#version 100\nvoid main(){gl_FragColor=vec4(gl_FragCoord.x,gl_FragCoord.y,.2,1.);}";


