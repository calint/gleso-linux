#include "../program16/main.hpp"

#include"../gleso/timer.hpp"
#include"../gleso/grid/wque.hpp"

#include<algorithm>
using namespace grid;

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

static void key_callback(GLFWwindow*window,int key,int scancode,int action,int mods){
	gleso_key(key,scancode,action,mods);
}

static float pointer_x=0,pointer_y=0;
static int pointer_button_action=1;
static void cursorpos_position_callback(GLFWwindow*window,double xpos,double ypos){
	pointer_x=(float)xpos;
	pointer_y=(float)ypos;
	if(pointer_button_action==0){// if button clicked, send drag event
		gleso_touch(pointer_x,pointer_y,2);
	}
}

static void mousebutton_callback(GLFWwindow*window,int button,int action,int mods){
	if(mods!=0)return;
	if(button!=GLFW_MOUSE_BUTTON_LEFT)
		return;
	switch(action){
	case GLFW_PRESS:
		pointer_button_action=0;
		gleso_touch(pointer_x,pointer_y,0);
		break;
	case GLFW_RELEASE:
		pointer_button_action=1;
		gleso_touch(pointer_x,pointer_y,1);
		break;
	}
}

static void windowsize_callback(GLFWwindow*window,int width,int height){
	gleso_viewport(width,height);
}

int main(int argc,char**argv){
//	test_threads();
//	test_wqueue(8);
//	exit(0);

	while(argc--)puts(*argv++);
//	atexit([]{gleso_deinit();});
	puts(glfwGetVersionString());
	GLFWwindow*window=nullptr;
	try{
		if(!glfwInit())throw"cannot init glfw";
	//	glfwWindowHint(GLFW_SAMPLES,4);
	//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	//	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
		window=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"gleso",NULL,NULL);
		if(!window){throw"cannot create window";}
		glfwSetKeyCallback(window,key_callback);
		glfwSetCursorPosCallback(window,cursorpos_position_callback);
		glfwSetMouseButtonCallback(window,mousebutton_callback);
		glfwSetWindowSizeCallback(window,windowsize_callback);
		glfwMakeContextCurrent(window);
	//	glewExperimental=GL_TRUE; // needed for core profile
		if(glewInit()!=GLEW_OK)throw"cannot init glew";
		gleso_init();
		gleso_viewport(WINDOW_WIDTH,WINDOW_HEIGHT);
		glfwSwapInterval(0);
		p("* running\n");
		timer tmr;
		while(!glfwWindowShouldClose(window)){
			auto dt=max(min(floato(1),tmr.dt()),floato(.0001));
			metric.dt=dt;
			gleso_step(dt);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}catch(const string&s){
		p("!!! exception: %s\n",s.c_str());
	}catch(const char*s){
		p("!!! exception: %s\n",s);
	}

	// omits deinit of gleso
	gleso_cleanup();

	if(window)glfwDestroyWindow(window);
	glfwTerminate();
	puts("* gleso exit");
	return 0;
}
