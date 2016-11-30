#pragma once
#include"cell.hpp"

namespace grid{
	class wque_work{
		cell&cell_;
		update_render_sync&urs_;
	public:
		inline wque_work(update_render_sync&urs,cell&c):cell_{c},urs_{urs}{}
		virtual~wque_work(){}
		inline void exec(){
			cell_.update_globs();
			cell_.handle_collisions();
			urs_.decrease_and_notify_if_zero();
		}
	};
}
