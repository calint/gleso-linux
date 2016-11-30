#pragma once
#include"cell.hpp"
#include"update_render_sync.hpp"
#include"wque_work.hpp"
#include"problems.hpp"

namespace grid{

	class wque_work_update_cell:public wque_work{
		cell&cell_;

		update_render_sync&urs_;

	public:

		inline wque_work_update_cell(update_render_sync&urs,cell&c):cell_{c},urs_{urs}{}

		inline virtual void exec()final{
			cell_.update_globs();
			cell_.handle_collisions();
			urs_.decrease_and_notify_if_zero();
		}

	};
}
