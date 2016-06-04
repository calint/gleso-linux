#pragma once
#include<cmath>
class m4{public:
	floato c[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
	m4&load_translate(const p3&p){
		// [ 0 4  8  x ]
		// [ 1 5  9  y ]
		// [ 2 6 10  z ]
		// [ 3 7 11 15 ]
		c[ 1]=c[ 2]=c[ 3]=c[ 4]=
		c[ 6]=c[ 7]=c[ 8]=c[ 9]=
		c[11]=0;
		c[ 0]=c[ 5]=c[10]=c[15]=1;
		c[12]=p.x;
		c[13]=p.y;
		c[14]=p.z;
		return*this;
	}
	m4&append_rotation_about_z_axis(const floato degrees){
		// [ 0 4  8 12 ]   [ cos -sin 0  0 ]
		// [ 1 5  9 13 ] x [ sin cos  0  0 ]
		// [ 2 6 10 14 ]   [ 0   0    1  0 ]
		// [ 3 7 11 15 ]   [ 0   0    0  1 ]
		const floato rad=degrees*floato(M_PI/180.0f);
		const floato cosrad = cosf(rad);
		const floato sinrad = sinf(rad);

		const floato mtx00=c[0];
		const floato mtx01=c[1];
		const floato mtx02=c[2];
		const floato mtx03=c[3];

		c[ 0]=c[ 4]*sinrad+mtx00*cosrad;
		c[ 4]=c[ 4]*cosrad-mtx00*sinrad;

		c[ 1]=c[ 5]*sinrad+mtx01*cosrad;
		c[ 5]=c[ 5]*cosrad-mtx01*sinrad;

		c[ 2]=c[ 6]*sinrad+mtx02*cosrad;
		c[ 6]=c[ 6]*cosrad-mtx02*sinrad;

		c[ 3]=c[ 7]*sinrad+mtx03*cosrad;
		c[ 7]=c[ 7]*cosrad-mtx03*sinrad;
		return*this;
	}
	m4&append_scaling(const p3&scale){
		// [ 0 4  8 12 ]   [ x 0 0 0 ]
		// [ 1 5  9 13 ] x [ 0 y 0 0 ]
		// [ 2 6 10 14 ]   [ 0 0 z 0 ]
		// [ 3 7 11 15 ]   [ 0 0 0 1 ]
		c[ 0]*=scale.x;
		c[ 4]*=scale.y;
		c[ 8]*=scale.z;

		c[ 1]*=scale.x;
		c[ 5]*=scale.y;
		c[ 9]*=scale.z;

		c[ 2]*=scale.x;
		c[ 6]*=scale.y;
		c[10]*=scale.z;

		c[ 3]*=scale.x;
		c[ 7]*=scale.y;
		c[11]*=scale.z;
		return*this;
	}
	m4&load_ortho_projection(const floato left,const floato right,const floato bottom,const floato top,const floato nearz,const floato farz){
		c[ 0]=2.f/(right-left);
		c[ 1]=0;
		c[ 2]=0;
		c[ 3]=0;

		c[ 4]=0;
		c[ 5]=2.f/(top-bottom);
		c[ 6]=0;
		c[ 7]=0;

		c[ 8]=0;
		c[ 9]=0;
		c[10]=-2.f/(farz-nearz);
		c[11]=0;

		c[12]=-(right+left)/(right-left);
		c[13]=-(top+bottom)/(top-bottom);
		c[14]=-(farz+nearz)/(farz-nearz);
		c[15]=1;
		return*this;
	}
};
m4 operator*(const m4&lh,const m4&rh){
	// [ 0 4  8 12 ]   [ 0 4  8 12 ]
	// [ 1 5  9 13 ] x [ 1 5  9 13 ]
	// [ 2 6 10 14 ]   [ 2 6 10 14 ]
	// [ 3 7 11 15 ]   [ 3 7 11 15 ]
	m4 m;
	floato*ret=m.c;
	const floato*lhs=lh.c;
	const floato*rhs=rh.c;
	ret[ 0] = lhs[ 0]*rhs[ 0] + lhs[ 4]*rhs[ 1] + lhs[ 8]*rhs[ 2] + lhs[12]*rhs[ 3];
	ret[ 1] = lhs[ 1]*rhs[ 0] + lhs[ 5]*rhs[ 1] + lhs[ 9]*rhs[ 2] + lhs[13]*rhs[ 3];
	ret[ 2] = lhs[ 2]*rhs[ 0] + lhs[ 6]*rhs[ 1] + lhs[10]*rhs[ 2] + lhs[14]*rhs[ 3];
	ret[ 3] = lhs[ 3]*rhs[ 0] + lhs[ 7]*rhs[ 1] + lhs[11]*rhs[ 2] + lhs[15]*rhs[ 3];

	ret[ 4] = lhs[ 0]*rhs[ 4] + lhs[ 4]*rhs[ 5] + lhs[ 8]*rhs[ 6] + lhs[12]*rhs[ 7];
	ret[ 5] = lhs[ 1]*rhs[ 4] + lhs[ 5]*rhs[ 5] + lhs[ 9]*rhs[ 6] + lhs[13]*rhs[ 7];
	ret[ 6] = lhs[ 2]*rhs[ 4] + lhs[ 6]*rhs[ 5] + lhs[10]*rhs[ 6] + lhs[14]*rhs[ 7];
	ret[ 7] = lhs[ 3]*rhs[ 4] + lhs[ 7]*rhs[ 5] + lhs[11]*rhs[ 6] + lhs[15]*rhs[ 7];

	ret[ 8] = lhs[ 0]*rhs[ 8] + lhs[ 4]*rhs[ 9] + lhs[ 8]*rhs[10] + lhs[12]*rhs[11];
	ret[ 9] = lhs[ 1]*rhs[ 8] + lhs[ 5]*rhs[ 9] + lhs[ 9]*rhs[10] + lhs[13]*rhs[11];
	ret[10] = lhs[ 2]*rhs[ 8] + lhs[ 6]*rhs[ 9] + lhs[10]*rhs[10] + lhs[14]*rhs[11];
	ret[11] = lhs[ 3]*rhs[ 8] + lhs[ 7]*rhs[ 9] + lhs[11]*rhs[10] + lhs[15]*rhs[11];

	ret[12] = lhs[ 0]*rhs[12] + lhs[ 4]*rhs[13] + lhs[ 8]*rhs[14] + lhs[12]*rhs[15];
	ret[13] = lhs[ 1]*rhs[12] + lhs[ 5]*rhs[13] + lhs[ 9]*rhs[14] + lhs[13]*rhs[15];
	ret[14] = lhs[ 2]*rhs[12] + lhs[ 6]*rhs[13] + lhs[10]*rhs[14] + lhs[14]*rhs[15];
	ret[15] = lhs[ 3]*rhs[12] + lhs[ 7]*rhs[13] + lhs[11]*rhs[14] + lhs[15]*rhs[15];
	return m;//? std::move
}
// - - - -------- - - - - - - -    - - --- - - - - - --- -- - - - - -
// forked from apple examples
