#pragma once
class glo_square_xyuvrgba:public glo_square_xyuv{
public:
//	glo_square_xyuvrgba(){
//		texture(nullptr);
//	}
	virtual vector<GLfloat>make_colors()const{
		vector<GLfloat>v;
		const GLfloat f[]{
		//   R  G  B A
			 0, 0, 0,1,
			 1, 1, 1,1,
			 0, 0, 0,1,
			-1,-1,-1,1,
		};
		v.assign(f,f+sizeof(f)/sizeof(GLfloat));
		return v;
	}
};
