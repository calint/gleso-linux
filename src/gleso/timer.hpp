#pragma once

namespace gleso{
	class timer{
		time_point<system_clock>t0;
	public:
		inline timer():t0(system_clock::now()){}

		inline floato dt(){
			const auto t1=system_clock::now();
			const auto dt=duration<floato,ratio<1>>(t1-t0).count();
			t0=t1;
			return dt;
		}

	};
}
