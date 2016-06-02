#pragma once
/*
     /////  ///\   ///// ///// /////////
     ///   ///\\\  ///   ///  // /// //
    ///   ///  \\\///   ///     ///
  ///// /////   \\\/  /////   /////
*/
#include"glo_ball.hpp"
#include"a_ball.hpp"
static void init(){
	using namespace gl;
	shaders.push_back(&shader::instance);
	textures.push_back(&texture::instance);//?? leak
	glos.push_back(&glo_circle_xy::instance);
	glos.push_back(&glo_grid::instance);
	glos.push_back(&glo_ball::instance);
	const int instances=16;
	for(int n=0;n<instances;n++)
		globs.push_back(new a_ball());
}








/*-----------------------------
     __    _       __     __
    /  \   |      /  \   /  \
   /       |     |    | |
   |  ___  |     |    |  \__
   |    |  |     |    |     \
    \___/  |___|  \__/   \__/

------------------------------
"defglo" */











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

































