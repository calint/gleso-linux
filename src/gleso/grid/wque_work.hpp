#pragma once

namespace grid{
	class wque_work{
	public:
		virtual~wque_work(){}
		virtual void exec()=0;
	};
}
