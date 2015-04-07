#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<math.h>
#include<string.h>
#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include "gleso.h"
static void key_callback(GLFWwindow*window,int key,int scancode,int action,int mods){
	if(window==0)return;//? unused param warning workaround
	gleso_key(key,scancode,action,mods);
}
static float pointer_x=0,pointer_y=0;
static int pointer_button_action=1;
static void cursorpos_position_callback(GLFWwindow*window,double xpos,double ypos){
	if(window==0)return;//? unused param warning workaround
	pointer_x=(float)xpos;
	pointer_y=(float)ypos;
	if(pointer_button_action==0){// if button clicked, send drag event
		gleso_touch(pointer_x,pointer_y,2);
	}
}
void mousebutton_callback(GLFWwindow*window,int button,int action,int mods){
	if(window==0)return;//? unused param warning workaround
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
	if(window==0)return;//? unused param warning workaround
	gleso_viewport(width,height);
}
#define WINDOW_WIDTH 240
#define WINDOW_HEIGHT 320
int main(int argc,char**argv){
	while(argc--)puts(*argv++);
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
		p("* running\n");
		while(!glfwWindowShouldClose(window)){
			gleso_step();
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}catch(const char*s){
		p("!!! exception: %s\n",s);
	}

	// omits deinit of gleso

	if(window)glfwDestroyWindow(window);
	glfwTerminate();
	puts("* gleso exit");
	return 0;
}
