#pragma once
#include"platform-linux/include.hpp"

#include<memory>
#include<cstdlib>
#include<vector>
#include<algorithm>
#include<chrono>
#include<cmath>
#include<atomic>

namespace gleso{
	bool render_globs{true};
	bool render_grid_outline{true};
	bool use_grid{true};
	bool update_grid_cells_in_parallell{true};

	class p3;

	typedef GLfloat floato;
	typedef long long longo;
	typedef longo frame_count;

	typedef p3 scale;
	typedef p3 angle;
	typedef p3 position;
	typedef p3 velocity_vector;
	typedef floato weight_kg;
	typedef floato radius_m;
	typedef floato time_s;

	inline floato rnd(floato min,floato max){return min+(floato(random())/RAND_MAX)*(max-min);}
}

using namespace std;
using namespace gleso;

