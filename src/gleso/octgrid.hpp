#pragma once
#include<algorithm>
#define foreach(c,f)std::for_each(c.begin(),c.end(),f)
class octgrid{
	p3 po;
	float s;
	vector<glob*>ls;
	vector<glob*>lsmx;
	octgrid*grds[8]{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
	const size_t splitthresh=1;
	const int subgridlevels=3;
public:
	inline octgrid(const floato size,const p3&p=p3{}):po(p),s(size){metrics::ngrids++;}
	inline~octgrid(){metrics::ngrids--;clear();}
	inline void clear(){
		ls.clear();
		lsmx.clear();
		for(auto&g:grds)
			if(g){
				g->clear();
				delete g;//? recycle
				g=nullptr;
			}
	}
	inline void addall(const vector<glob*>&ls){
		for(auto g:ls){
			g->grid_that_updates_this_glob=nullptr;
			putif(g,g->phy.p,g->phy.r);
		}
		splitif(subgridlevels);//? splititonthefly
		//? ifallglobswhereaddedtoallsubgrids,stoprecurtion
	}
	inline void update_globs(){//! multicore
//		p(" update grid %p\n",(void*)this);
		for(auto g:ls){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->update();
		}
		for(auto g:lsmx){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->update();
		}
		for(auto&g:grds)
			if(g)
				g->update_globs();
	}
	inline void render_globs(){
		for(auto g:ls){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->render();
		}
		for(auto g:lsmx){
			if(g->grid_that_updates_this_glob!=this)
				continue;
			g->render();
		}
		for(auto&g:grds)
			if(g)
				g->render_globs();

	}
	inline void render_outline(){
		bool rendered=false;
		for(auto gr:grds){
			if(!gr)
				continue;
			gr->render_outline();
			rendered=true;
		}
		if(rendered)return;
//		if(ls.empty())return;
		m4 m;
		m.load_translate(po);
		m.append_scaling(p3{s,s,s});
		glUniformMatrix4fv(GLint(gl::umtx_mw),1,false,m.c);
		glo_grid::instance.render();
	}
//	void coldet(){
//		if(!ls.empty()){
//			auto i1=ls.begin();
//			while(true){
//				auto i2=ls.rbegin();
//				if(*i1==*i2)
//					break;
//				glob&g1=*(*i1);
//				do{	glob&g2=*(*i2);
//					g1.coldet(g2);
//					i2++;
//				}while(*i1!=*i2);
//				i1++;
//			}
//			if(!lsmx.empty())
//				for(auto g1:ls)
//					for(auto g2:lsmx)
//						g1->coldet(*g2);
//		}
//		for(auto g:grds)
//			if(g)
//				g->coldet();
//	}
//	void culldraw(const bvol&bv){
//		const int c=bv.cull(po,s*1.41f);//? radius
//		if(c){
//			metrics::gridsculled++;
//			return;
//		}
//		metrics::gridsrend++;
//		for(auto g:ls){glPushMatrix();g->culldraw(bv);glPopMatrix();}
//		for(auto g:lsmx){glPushMatrix();g->culldraw(bv);glPopMatrix();}
//		for(auto&g:grds)
//			if(g)
//				g->culldraw(bv);
//	}
private:
	inline bool putif(glob*g,const p3&p,const floato r){
		if((p.x+s+r)<po.x)return false;
		if((p.x-s-r)>po.x)return false;
		if((p.z+s+r)<po.z)return false;
		if((p.z-s-r)>po.z)return false;
		if((p.y+s+r)<po.y)return false;
		if((p.y-s-r)>po.y)return false;
//		if(g->iscolmx()){
//			lsmx.push_back(g);
//		}else
			ls.push_back(g);
		if(!g->grid_that_updates_this_glob)
			g->grid_that_updates_this_glob=this;
		return true;
	}
	inline bool splitif(const int nrec){
//		if((ls.size())<splitthresh)//? alg
		if((ls.size()+lsmx.size())<splitthresh)//? alg
			return false;
		if(nrec==0)
			return false;
		const float ns=s/2;
		grds[0]=new octgrid(ns,po+p3{-ns,ns,-ns});//?
		grds[1]=new octgrid(ns,po+p3{ ns,ns,-ns});
		grds[2]=new octgrid(ns,po+p3{-ns,ns, ns});
		grds[3]=new octgrid(ns,po+p3{ ns,ns, ns});

		grds[4]=new octgrid(ns,po+p3{-ns,-ns,-ns});
		grds[5]=new octgrid(ns,po+p3{ ns,-ns,-ns});
		grds[6]=new octgrid(ns,po+p3{-ns,-ns, ns});
		grds[7]=new octgrid(ns,po+p3{ ns,-ns, ns});

		for(auto o:ls)
			o->grid_that_updates_this_glob=nullptr;
		for(auto o:lsmx)
			o->grid_that_updates_this_glob=nullptr;

		for(auto g:grds){
			for(auto o:ls){
				g->putif(o,o->phy.p,o->phy.r);//?
			}
			for(auto o:lsmx){
				g->putif(o,o->phy.p,o->phy.r);//?
			}
			g->splitif(nrec-1);//?
		}
		ls.clear();
		lsmx.clear();
		return true;
	}
};
static octgrid grd(1);
