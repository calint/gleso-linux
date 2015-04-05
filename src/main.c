#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<math.h>
#include<string.h>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include"gleso.h"
static void key_callback(GLFWwindow*window,int key,int scancode,int action,int mods){
	gleso_key(key,scancode,action,mods);
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
