#pragma once
#include"../platform.hpp"
#include"a_sphere.hpp"
	using namespace gl;

static void test0(){
	const int instances=1024*8;
	p(" > instances: %d\n",instances);
	for(int n=0;n<instances;n++){
		globs.push_back(new a_ball());
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
	globs.push_back(new a_sphere{{.2},.05,{ .05}});
	globs.push_back(new a_sphere{{.4},.05,{-.05}});
}

static void init(){
	shaders.push_back(&shader::instance);
	textures.push_back(&texture::instance);
//	glos.push_back(&glo_circle_xy::instance);
	glos.push_back(&glo_grid::instance);
	glos.push_back(&glo_ball::instance);

//	test0();
//	test1();
//	test2();
//	test3();
//	test4();
	test5();
//	test6();
//	test7();
//	test8();
//	test9();
}
