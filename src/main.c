#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gles.h"
#include <GL/glu.h>
#include <GL/glut.h>
static void _display(void){
	gleso_step();
	glutSwapBuffers();
}
static void _reshape(int w, int h){
	glViewport(0,0,w,h);
}
static void _idle(){
	glutPostRedisplay();
}
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL);
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(200, 100);
  glutCreateWindow(argv[0]);
  gleso_init();
  glutDisplayFunc(_display);
  glutReshapeFunc(_reshape);
  glutIdleFunc(_idle);
  glutMainLoop();
  return 0;
}
