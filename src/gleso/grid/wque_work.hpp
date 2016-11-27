#pragma once
#include"cell.hpp"
#include"update_render_sync.hpp"

namespace grid{
	class wque_work{
	public:
		virtual~wque_work(){}
		virtual void exec()=0;
	};
}
