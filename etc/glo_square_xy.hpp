#pragma once
class glo_square_xy:public glo{
	virtual vector<GLfloat>make_vertices()const{
		const static GLfloat verts[]={-1,1, -1,-1, 1,-1, 1,1};
		vector<GLfloat>v;//{-1,1, -1,-1, 1,-1, 1,1};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		vector<GLfloat>v;
//		const static GLfloat verts[]={-1,1, -1,-1, 1,-1, 1,1};
//		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	inline virtual void gldraw()const{
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
	}
};
