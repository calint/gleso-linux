#pragma once
#include"cell.hpp"
#include"update_render_sync.hpp"
#include "wque_work.hpp"

namespace grid{

	class wque_work_update_cell:public wque_work{
		cell*cell_;
	public:
		wque_work_update_cell(cell*c):cell_{c}{}

		virtual void exec()final{
			cell_->update_globs();
			update_render_sync::work_to_do_count::decrease_and_notify_if_zero();
		}
	};

}
