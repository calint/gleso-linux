#pragma once
#include"cell.hpp"

namespace grid{
	class wque_work{

		cell&c_;

		update_render_sync&s_;

	public:

		inline wque_work(update_render_sync&urs,cell&c):c_{c},s_{urs}{}

		inline void exec(){

			c_.update_globs();

			c_.handle_collisions();

			s_.decrease_and_notify_if_zero();

		}

	};
}
