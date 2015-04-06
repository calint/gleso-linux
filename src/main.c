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
static void cursor_position_callback(GLFWwindow*window,double xpos,double ypos){
	if(window==0)return;//? unused param warning workaround
	pointer_x=(float)xpos;
	pointer_y=(float)ypos;
	if(pointer_button_action==0){// if button clicked, send drag event
		gleso_touch(pointer_x,pointer_y,2);
	}
}
void mouse_button_callback(GLFWwindow*window,int button,int action,int mods){
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
	gleso_on_viewport_change(width,height);
}
#define WINDOW_WIDTH 240
#define WINDOW_HEIGHT 320
int main(int argc,char**argv){
	while(argc--)puts(*argv++);
	puts(glfwGetVersionString());
	if(!glfwInit())return 1;
//	glfwWindowHint(GLFW_SAMPLES,4);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow*window=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"gleso",NULL,NULL);
	if(!window){glfwTerminate();return 2;}
	glfwSetKeyCallback(window,key_callback);
	glfwSetCursorPosCallback(window,cursor_position_callback);
	glfwSetMouseButtonCallback(window,mouse_button_callback);
	glfwSetWindowSizeCallback(window,windowsize_callback);
	glfwMakeContextCurrent(window);
//	glewExperimental=GL_TRUE; // needed for core profile
	if (glewInit()!=GLEW_OK)return 3;
	gleso_init();
	gleso_on_viewport_change(WINDOW_WIDTH,WINDOW_HEIGHT);
	while(!glfwWindowShouldClose(window)){
		gleso_step();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	puts("done");
	return 0;
}
