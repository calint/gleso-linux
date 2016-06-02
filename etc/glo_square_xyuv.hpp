#pragma once
class glo_square_xyuv:public glo{
public:
	glo_square_xyuv(){
		set_texture(&texture::instance);
	}
	virtual vector<GLfloat>make_vertices()const{
		vector<GLfloat>v;
		const static GLfloat verts[]{
		//	 x  y
			-1, 1,
			-1,-1,
			 1,-1,
			 1, 1
		};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	virtual vector<GLfloat>make_texture_coords()const{
		vector<GLfloat>v;
		const static GLfloat verts[]{
		//	u v
			0,1,
			0,0,
			1,0,
			1,1
		};
		v.assign(verts,verts+sizeof(verts)/sizeof(GLfloat));
		return v;
	}
	inline virtual void gldraw()const{
//		texels_rgb[0]++;
//		texels_rgb[1]++;
		get_texture_for_update().refresh_from_data();
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
	}
	virtual vector<GLfloat>make_colors()const{
		return vector<GLfloat>();
	}
};
