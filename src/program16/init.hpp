#pragma once
#include"../include.hpp"
#include"../gleso/grid/glo_grid.hpp"
#include"a_ball.hpp"
#include"a_sphere.hpp"
#include"a_static_sphere.hpp"
#include"a_bullet.hpp"
using namespace grid;
using namespace program16;

static void test0(){
	const int instances=1024*8;
	p(" > instances: %d\n",instances);
	for(int n=0;n<instances;n++){
		globs.push_back(new a_ball);
	}
}

static void test1(){
	globs.push_back(new a_sphere);
}

static void test2(){
	globs.push_back(new a_sphere{{-.3}});
	globs.push_back(new a_sphere{{ .3}});
}

static void test3(){
	globs.push_back(new a_sphere{{.1,.3},.05});
	globs.push_back(new a_sphere{{.4,.3},.05});
}

static void test4(){
	globs.push_back(new a_sphere{{.1,.3},.05,{ .02}});
	globs.push_back(new a_sphere{{.4,.3},.05,{-.02}});
}

static void test5(){
	globs.push_back(new a_sphere{{.2,.3},.05,{ .1}});
	globs.push_back(new a_sphere{{.4,.3},.05,{-.1}});
}

static void test6(){
	globs.push_back(new a_sphere{{.2},.05,{ .1}});
	globs.push_back(new a_sphere{{.4},.05,{-.1}});
}

static void test7(){
	globs.push_back(new a_sphere{{0,-.3},.05,{0,.1}});
}

static void test8(){
	globs.push_back(new a_sphere{{},.05});
}

static void test9(){
	globs.push_back(new a_sphere{.2,.05,.1});
	globs.push_back(new a_static_sphere{.5,.05});
}

static void test10(){
	for(int i=0;i<1024;i++)
		globs.push_back(new a_ball);
}

static void test11(){
	const floato constraint=1;
	const floato velocity=.1;
	for(int i=0;i<1024;i++){
		p3 po;
		po.x=rnd(-constraint,constraint);
		po.y=rnd(-constraint,constraint);
		const floato r=.01;
		velocity_vector v;
		v.x=rnd(-velocity,velocity);
		v.y=rnd(-velocity,velocity);

		globs.push_back(new a_sphere(po,r,v,constraint));
	}
}

static void test12(){
	const floato constraint=1;
	const floato velocity=.05;
	const int count=1024*8;
//	gleso::render_globs=false;
//	gleso::render_grid_outline=false;

	const floato r=.005;
	for(int i=0;i<count;i++){
		p3 po{rnd(-constraint,constraint),rnd(-constraint,constraint)};
		velocity_vector v{rnd(-velocity,velocity),rnd(-velocity,velocity)};
		globs.push_back(new a_sphere(po,r,v,constraint));
	}
}

static void test13(){
	const floato constraint=1;
	const int count=8;
	for(int i=0;i<count;i++){
		p3 po{rnd(-constraint,constraint),rnd(-constraint,constraint)};
		const floato r=.05+rnd(0,.2);
		globs.push_back(new a_static_sphere(po,r));
	}
}

static void test14(){
	const floato constraint=1;
	const floato velocity=.05;
	const int count=1024*2;
//	render_globs=false;

	const floato r=.02;
	for(int i=0;i<count;i++){
		p3 po{rnd(-constraint,constraint),rnd(-constraint,constraint)};
		velocity_vector v{rnd(-velocity,velocity),rnd(-velocity,velocity)};
		globs.push_back(new a_sphere(po,r,v,constraint));
	}
}

















static void init(){
	shaders.push_back(&shader::instance);
	textures.push_back(&texture::instance);
//	glos.push_back(&glo_circle_xy::instance);
	glos.push_back(&glo_grid::instance);
	glos.push_back(&glo_ball::instance);
	glos.push_back(&glo::instance);
	glos.push_back(&glo_camera::instance);
	glos.push_back(&glo_bullet::instance);

//	test0();
//	test1();
//	test2();
//	test3();
//	test4();
//	test5();
//	test6();
//	test7();
//	test8();
//	test9();
//	test10();
//	test11();
//	test12();
//	test13();
	test14();
}

