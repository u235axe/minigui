#include <numeric>
#include "graphics_base.h"
#include "miniwindow.h"
#include "rendertext.h"

namespace UI2
{
	struct Style
	{
		StbFont font;
		Color8  bg, fg;
		float   height;
	};

	enum class HContentAlign { Fill = 255, Left = 2, Center, Right  };
	enum class VContentAlign { Fill = 255, Top  = 2, Center, Bottom };
	enum class Sizing { TopDown,  BottomUp };

	void alignContentToOuter(rect2i& content, rect2i const& outer, size2i gap, HContentAlign hca, VContentAlign vca)
	{
		if     (hca == HContentAlign::Left  ){ content.x = left   (outer) + gap.w; }
		else if(hca == HContentAlign::Center){ content.x = hcenter(outer) - content.w/2;         }
		else if(hca == HContentAlign::Right ){ content.x = right  (outer) - gap.w - content.w; }
		else if(hca == HContentAlign::Fill  ){ content.x = left   (outer) + gap.w; content.w = outer.w - 2*gap.w; }

		if     (vca == VContentAlign::Top   ){ content.y = top    (outer) + gap.h; }
		else if(vca == VContentAlign::Center){ content.y = vcenter(outer) - content.h/2;         }
		else if(vca == VContentAlign::Bottom){ content.y = bottom (outer) - gap.h - content.h; }
		else if(vca == VContentAlign::Fill  ){ content.y = top(outer) + gap.h; content.h = outer.h - 2*gap.h; }
	}

	template<typename T> struct ValueProxy;
	template<typename T> struct ValueRenderer;
	template<typename T> struct MultiValueRenderer;

	template<typename T>
	struct ValueRendererBase
	{
		Image2<Color8> bitmap;
		T*     p;
		Style* s;

		ValueRendererBase():p{nullptr}, s{nullptr}{}
		void setStyle (Style& s_){ s = &s_; } 
		void setTarget(T&     p_){ p = &p_; }

		virtual int    nElems() const { return 0; }
		virtual void   update(){}
		virtual size2i getSize() const { return {0,0}; }
		virtual void   draw(rect2i rct, SoftwareRenderer& sr){}
	};

	template<> struct ValueRenderer<int> : ValueRendererBase<int>
	{
		void update()
		{
			if(p && s)
			{
				utf8string str(*p);
				auto pt = render_small_string_monospace(str, s->font, s->height);
				bitmap = recolor(pt.img, s->bg, s->fg);
			}
		}

		int    nElems()  const { return 1; }
		size2i getSize() const { return bitmap.size(); }

		void draw(rect2i rct, SoftwareRenderer& sr)
		{
			sr.copy_image(bitmap, rct.x, rct.y);
		}
	};

	template<> struct ValueRenderer<utf32string> : ValueRendererBase<utf32string>
	{
		void update()
		{
			if(p && s)
			{
				auto pt = render_small_string_monospace(*p, s->font, s->height);
				bitmap = recolor(pt.img, s->bg, s->fg);
			}
		}

		int    nElems()  const { return 1; }
		size2i getSize() const { return bitmap.size(); }

		void draw(rect2i rct, SoftwareRenderer& sr)
		{
			sr.copy_image(bitmap, rct.x, rct.y);
		}
	};

	struct ValueProxyBase
	{
		virtual void   update(){}
		virtual int    nElems() const { return 0; }
		virtual size2i getSize() const { return {0,0}; }
		virtual void   draw(rect2i rct, SoftwareRenderer& sr){}
		virtual ~ValueProxyBase(){}
	};

	template<typename T>
	struct ValueProxy : ValueProxyBase
	{
		ValueRenderer<T> r;
		int    nElems() const { return r.nElems(); }
		void   setTarget(T&     v){ r.setTarget(v); }
		void   setStyle(Style& s){ r.setStyle(s); }
		void   update(){ r.update(); }
		size2i getSize() const { return r.getSize(); }
		void   draw(rect2i rct, SoftwareRenderer& sr){ r.draw(rct, sr); }
		~ValueProxy(){}
	};

	struct MultiValueProxyBase
	{
		virtual void   update(){}
		virtual int    nElems() const { return 0; }
		virtual size2i getElemSize(int i) const { return {0,0}; }
		virtual void   drawElem(int i, rect2i rct, SoftwareRenderer& sr){}
		virtual ~MultiValueProxyBase(){}
	};

	template<typename T>
	struct MultiValueRendererBase
	{
		std::vector<Image2<Color8>> bitmap;
		T*     p;
		Style* s;

		MultiValueRendererBase():p{nullptr}, s{nullptr}{}
		void setStyle (Style& s_){ s = &s_; } 
		void setTarget(T&     p_){ p = &p_; }

		virtual int    nElems() const { return 0; }
		virtual void   update(){}
		virtual size2i getElemSize(int i) const { return {0,0}; }
		virtual void   drawElem(int i, rect2i rct, SoftwareRenderer& sr){}
		virtual ~MultiValueRendererBase(){}
	};

	template<> struct MultiValueRenderer<std::vector<int>> : MultiValueRendererBase<std::vector<int>>
	{
		void update() override
		{
			if(p && s)
			{
				int n = (int)p->size();
				bitmap.resize( n );
				for(int i=0; i<n; ++i)
				{
					utf8string str((*p)[i]);
					auto pt = render_small_string_monospace(str, s->font, s->height);
					bitmap[i] = recolor(pt.img, s->bg, s->fg);
				}
			}
		}

		int    nElems()  const override { return (p ? (int)p->size() : 0); }
		size2i getElemSize(int i) const override
		{
			auto sz = bitmap[i].size();
			//std::cout << i << " size: " << sz.w << ", " << sz.h << "\n";
			return sz;
		}

		void drawElem(int idx, rect2i rct, SoftwareRenderer& sr) override
		{
			if(s)
			{
				sr.filledrect(rct, s->bg);
			}
			sr.copy_image(bitmap[idx], rct.x, rct.y);
		}
	};

	template<typename T>
	struct MultiValueProxy : MultiValueProxyBase
	{
		MultiValueRenderer<T> r;
		int    nElems() const override { return r.nElems(); }
		void   setTarget(T&     v){ r.setTarget(v); }
		void   setStyle(Style& s){ r.setStyle(s); }
		void   update()override{ r.update(); }
		size2i getElemSize(int i) const override { return r.getElemSize(i); }
		void   drawElem(int i, rect2i rct, SoftwareRenderer& sr)override{ r.drawElem(i, rct, sr); }
	};
	
	struct Base
	{
		rect2i rect;
		rect2i content;
		size2i gap;
		HContentAlign cha;//content rect align w.r.t the outer rect
		VContentAlign cva;//content rect align w.r.t the outer rect
		HContentAlign icha;//internal content alignment w.r.t content rect
		VContentAlign icva;//internal content alignment w.r.t content rect
		Sizing        sz;
		bool updateRq, realignRq;

		void alignContentAndRect(pos2i pos, size2i outersz)
		{
			alignContentToOuter(rect, rect2i{pos.x, pos.y, outersz.w, outersz.h}, {0,0}, cha, cva);
			alignContentToOuter(content, rect, gap, cha, cva);
		}

		virtual size2i getPreferredSize() const{ return {0,0}; }
		virtual void updateContent(){}
		virtual void realign(pos2i pos, size2i outersz){}
		virtual void draw(SoftwareRenderer& sr){}
	};

	struct Leaf : Base
	{
		std::shared_ptr<ValueProxyBase> proxy;

		Leaf(){ gap = {8,8}; cha = icha = HContentAlign::Center; cva = icva = VContentAlign::Bottom; sz = Sizing::BottomUp; updateRq = true; realignRq = true; }

		void setProxy(std::shared_ptr<ValueProxyBase> p){ proxy = p; }
	
		virtual size2i getPreferredSize() const { return (proxy ? proxy->getSize() : size2i{0,0}) + 2*gap; }

		virtual void updateContent()
		{
			if(proxy){ proxy->update(); }
			if(sz == Sizing::BottomUp){ content = getPreferredSize() - 2*gap; rect = center_shrink(content, -gap.w, -gap.h); }
			else                      { content = rect.size() - 2*gap; }
			updateRq = false; realignRq = true;
		}

		virtual void realign(pos2i pos, size2i outersz)
		{
			alignContentAndRect(pos, outersz);
			realignRq = false;
		}

		virtual void draw(SoftwareRenderer& sr)
		{
			sr.framedrect(rect, color8(192,192,192), color8(128,128,128));
			sr.rect(content, color8(255,0,0));
			if(proxy){ proxy->draw(content, sr); }
		}
	};

	template<typename T>
	auto view_value(T& t, Style& s)
	{
		ValueProxy<T> vp; vp.setTarget(t); vp.setStyle(s);
		return std::make_shared<ValueProxy<T>>(std::move(vp));
	}

	struct ListBounds
	{
		int W, H, maxW, maxH;
		ListBounds(){ reset(); }
		void reset(){ W = H = maxW = maxH = 0; }
	};

	template<typename Fidx>
	ListBounds computeBoundsOf(int n, Fidx&& fidx)
	{
		ListBounds lb;
		for(int i=0; i<n; ++i)
		{
			auto sz = fidx(i);
			lb.maxW = std::max(lb.maxW, sz.w);
			lb.maxH = std::max(lb.maxH, sz.h);
			lb.W += sz.w;
			lb.H += sz.h;
		}
		return lb;
	}

	struct ListBase : Base
	{
		int elemgap;
		bool isHorizontal;

		ListBase():elemgap{0}, isHorizontal{true}{}

		size2i eqDivSize() const
		{
			int n = nElems();
			if(n == 0){ return {0,0}; }
			size2i szc = content.size();
			int g = (n-1)*elemgap;
			if(isHorizontal){ szc.w = (content.w - g) / n; }else{ szc.h = (content.h - g) / n; }
			return szc;
		}

		virtual int    nElems() const { return 0; }
		virtual Base*  getElem(int i){ return nullptr; }
		virtual void   updateElem(int i){}
		virtual size2i getElemSize(int i) const { return {0,0}; }
		virtual size2i getPreferredSize() const override
		{
			int n = nElems();
			auto b = computeBoundsOf(n, [&](int i){ return getElemSize(i); });
			return size2i{isHorizontal ? b.W + (n-1)*elemgap : b.maxW, isHorizontal ? b.maxH : b.H + (n-1)*elemgap} + 2*gap;
		};
	};

	template<typename F>
	void alignChildAsList(ListBase& lb, F&& fa)
	{
		auto eqsz = lb.eqDivSize();
		auto p = lb.content.pos();
		auto n = lb.nElems();
		for(int i=0; i<n; ++i)
		{
			Base* c = lb.getElem(i);
			if(lb.isHorizontal)
			{
				int d = (lb.sz == Sizing::TopDown ? eqsz.w : c->rect.w);
				fa(c, p, size2i{d, lb.content.h});
				p.x += d + lb.elemgap;
			}
			else
			{
				int d = (lb.sz == Sizing::TopDown ? eqsz.h : c->rect.h);
				fa(c, p, size2i{lb.content.w, d});
				p.y += d + lb.elemgap;
			}
		}
	}

	template<typename F>
	void boundChilds(ListBase& lb, F&& fupdate)
	{
		auto n = lb.nElems();
		if(lb.sz == Sizing::BottomUp)
		{
			lb.content = lb.getPreferredSize() - 2*lb.gap;
			lb.rect    = center_shrink(lb.content, -lb.gap.w, -lb.gap.h);
		}
		else
		{
			lb.content = lb.rect.size() - 2*lb.gap;
			auto eqsz  = lb.eqDivSize();
			for(int i=0; i<n; ++i)
			{
				auto c = lb.getElem(i);
				c->rect = assignL(lb.getElemSize(i), eqsz);
				fupdate(c);
			}
		}
	}

	struct List : ListBase
	{
		std::vector<Base*> childs;

		List()
		{
			gap = {8,8}; elemgap = 4; cha = HContentAlign::Center; cva = VContentAlign::Center; sz = Sizing::BottomUp; updateRq = true; realignRq = true; isHorizontal = false;
			icva = VContentAlign::Center; icha = HContentAlign::Fill;
		}

		int    nElems() const override { return (int)childs.size(); }
		Base*  getElem(int i) override { return childs[i]; }
		void   updateElem (int i) override { childs[i]->updateContent(); };
		size2i getElemSize(int i) const override { return childs[i]->getPreferredSize(); }
		void   add( Base& x ){ childs.push_back(&x); }

		void updateContent()
		{
			for(int i=0; i<nElems(); ++i){ auto c = getElem(i); c->cha = icha; c->cva = icva; c->sz = sz; c->updateContent(); }
			boundChilds(*this, [](Base* c){ c->updateContent(); });
			updateRq = false; realignRq = true;
		}

		void realign(pos2i pos, size2i outersz)
		{
			alignContentAndRect(pos, outersz);
			alignChildAsList(*this, [](Base* c, pos2i p, size2i s){ c->realign(p, s); });
			realignRq = false;
		}

		void draw(SoftwareRenderer& sr)
		{
			sr.framedrect(rect, color8(192,192,192), color8(64,64,64));
			for(auto& c : childs){ c->draw(sr); }
			//sr.rect(content, color8(255,0,255));
		}
	};

	struct ListData : ListBase
	{
		std::shared_ptr<MultiValueProxyBase> proxy;
		Base              reference;
		std::vector<Base> chs;

		int    nElems() const override { return proxy ? proxy->nElems() : 0; }
		Base*  getElem(int i) override { return &(chs[i]); }
		void   updateElem(int i) override {}
		size2i getElemSize(int i) const override { return (proxy ? proxy->getElemSize(i) : size2i{0,0}) + 2*reference.gap; }

		ListData()
		{
			gap = {8,8}; elemgap = 2; icha = cha = HContentAlign::Center; cva = VContentAlign::Top; sz = Sizing::BottomUp; updateRq = true; realignRq = true; isHorizontal = false;
			reference.gap = {1,1}; reference.cha = HContentAlign::Fill; icva = reference.cva = VContentAlign::Center; reference.sz = Sizing::BottomUp;
			reference.updateRq = true; reference.realignRq = true;
		}

		void setProxy(std::shared_ptr<MultiValueProxyBase> p){ proxy = p; }

		void updateContent()override
		{
			if(proxy)
			{
				proxy->update();
				chs.resize(proxy->nElems());
				int n = nElems();
				//construct placeholder elements based on reference for later alignment, with sizes corresponding to the actual elements:
				for(int i=0; i<n; ++i)
				{
					auto& c = chs[i];
					c = reference;
					c.sz = sz;
					c.content = proxy->getElemSize(i);
					if(c.sz == Sizing::BottomUp){ c.rect = center_shrink(c.content, -c.gap.w, -c.gap.h); }
				}
				boundChilds(*this, [](Base* c){ c->content = c->rect.size() - 2*c->gap; });
			}
			updateRq = false; realignRq = true;
		}

		void realign(pos2i pos, size2i outersz)override
		{
			alignContentAndRect(pos, outersz);
			alignChildAsList(*this, [](Base* c, pos2i p, size2i s){ c->alignContentAndRect(p, s); });
			realignRq = false;
		}

		void draw(SoftwareRenderer& sr)override
		{
			sr.framedrect(rect, color8(192,192,192), color8(64,64,64));
			if(proxy)
			{
				int n = nElems();
				for(int i=0; i<n; ++i){ proxy->drawElem(i, chs[i].content, sr); }
			}
			//sr.rect(content, color8(255,0,255));
		}
	};

	template<typename T>
	auto view_multi_value(T& t, Style& s)
	{
		MultiValueProxy<T> vp; vp.setTarget(t); vp.setStyle(s);
		return std::make_shared<MultiValueProxy<T>>(vp);
	}
}