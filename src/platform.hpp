#pragma once
#include"platform-linux/platform.hpp"
#include<memory>
#include<cstdlib>
#include<vector>
#include<algorithm>
using namespace std;


typedef GLfloat floato;
typedef long long longo;


typedef floato weight_kg;
typedef floato radius_m;

typedef int count;

class p3;
typedef p3 scale;
typedef p3 angle;
typedef p3 position;
typedef p3 velocity_vector;
typedef longo frame_count;

//#define foreach(c,f)std::for_each(c.begin(),c.end(),f);

inline floato rnd(floato min,floato max){return min+(floato(random())/RAND_MAX)*(max-min);}

#include"gleso/gleso.hpp"
