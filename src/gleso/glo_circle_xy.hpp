#pragma once
class glo_circle_xy:public glo{
	int nvertices;
public:
	inline glo_circle_xy():nvertices(1+12+1){}
protected:
	vector<GLfloat>make_vertices()const override{
		vector<GLfloat>v;
		v.push_back(0);//x
		v.push_back(0);//y
		floato rad=0;
		const floato drad=2*float(M_PI/12);
		for(int i=1;i<=nvertices;i++){
			const floato x=cosf(rad);
			const floato y=sinf(rad);
			rad+=drad;
			v.push_back(x);
			v.push_back(y);
		}
		return v;
	}
	inline void gldraw()const override{glDrawArrays(GL_TRIANGLE_FAN,0,nvertices);}
public:
	static glo_circle_xy instance;
};
glo_circle_xy glo_circle_xy::instance=glo_circle_xy();
