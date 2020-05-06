#include <numeric>
#include <memory>
#include <optional>
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

	template<typename T>
	auto view_value(T& t, Style& s)
	{
		ValueProxy<T> vp; vp.setTarget(t); vp.setStyle(s);
		return std::make_shared<ValueProxy<T>>(std::move(vp));
	}

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
	
	template<typename T>
	auto view_multi_value(T& t, Style& s)
	{
		MultiValueProxy<T> vp; vp.setTarget(t); vp.setStyle(s);
		return std::make_shared<MultiValueProxy<T>>(vp);
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

	struct Layout
	{
		rect2i rect;
		rect2i content;
		size2i gap;
		HContentAlign cha;//content rect align w.r.t the outer rect
		VContentAlign cva;//content rect align w.r.t the outer rect
		HContentAlign icha;//internal content alignment w.r.t content rect
		VContentAlign icva;//internal content alignment w.r.t content rect
		Sizing        sz;

		Layout()
		{
			rect.zero(); content.zero(); gap = {1,1};
			cha = icha = HContentAlign::Center; cva = icva = VContentAlign::Center; sz = Sizing::BottomUp;
		}

		void alignContentAndRect(pos2i pos, size2i outersz)
		{
			alignContentToOuter(rect, rect2i{pos.x, pos.y, outersz.w, outersz.h}, {0,0}, cha, cva);
			alignContentToOuter(content, rect, gap, cha, cva);
		}

		void updateContent(size2i contentSize)
		{
			if(sz == Sizing::BottomUp){ content = contentSize; rect = center_shrink(content, -gap.w, -gap.h); }
			else                      { content = rect.size() - 2*gap; }
		}

		using ChL  = std::function<Layout*(int)>;
		using ChSz = std::function<size2i (int)>;
		using ChUp = std::function<void   (int)>;
		using ChAl = std::function<void   (int, pos2i, size2i)>;

		virtual size2i getContentSize(int n,                            ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) const{ return {0,0}; };
		virtual void   update        (int n,                            ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz){}
		virtual void   realign       (int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign){}
	};

	struct SingeElementLayout : Layout
	{
		SingeElementLayout(){}
		SingeElementLayout(SingeElementLayout const& cpy){ *this = cpy; }
		size2i getContentSize(int n, ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) const override
		{
			if(selfprefsz){ return selfprefsz.value() - 2*gap; } return {0,0};
		};

		void update (int n, ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) override
		{
			if( selfprefsz ){ updateContent(selfprefsz.value() - 2*gap); }
		}

		void realign(int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign) override
		{
			alignContentAndRect(pos, outersz);
		}
	};

	struct Col2Layout : Layout
	{
		size2i getContentSize(int n, ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) const override
		{
			size2i sz0 = chsize(0); size2i sz1 = chsize(1);
			return size2i{std::max(sz0.w, sz1.w), sz0.h + sz1.h};
		};

		void update (int n, ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) override
		{
			updateContent(getContentSize(n, chl, chsize, chupdate, std::nullopt));
		}

		void realign(int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign) override
		{
			alignContentAndRect(pos, outersz);

			size2i sz0 = chsize(0);
			size2i sz1 = chsize(1);
			rect2i r0; r0 = size2i{std::max(sz0.w, sz1.w), sz0.h + sz1.h};
			alignContentToOuter(r0, content, {0,0}, icha, icva);
			rect2i r;
			r = sz0;
			alignContentToOuter(r, r0, {0,0}, icha, VContentAlign::Top);
			chl(0)->rect = r;
			r = sz1;
			alignContentToOuter(r, r0, {0,0}, icha, VContentAlign::Bottom);
			chl(1)->rect = r;
		}
	};

	struct ListLayout: Layout
	{
		int elemgap;
		bool isHorizontal;

		//could be global?
		void boundChilds(int n, size2i contentSize, ChL chl, ChSz chsz, ChUp chup)
		{
			if(sz == Sizing::BottomUp)
			{
				content = contentSize;//lb.getPreferredSize() - 2*gap;
				rect    = center_shrink(content, -gap.w, -gap.h);
			}
			else
			{
				content = rect.size() - 2*gap;
				auto eqsz  = eqDivSize(n);
				for(int i=0; i<n; ++i)
				{
					auto cl = chl(i);
					cl->rect = assignL(chsz(i), eqsz);
					chup(i);
				}
			}
		}

		//could be global?
		void alignChildAsList(int n, ChL chl, ChAl chalign)
		{
			auto eqsz = eqDivSize(n);
			auto p = content.pos();
			for(int i=0; i<n; ++i)
			{
				Layout* cl = chl(i);
				if(isHorizontal)
				{
					int d = (sz == Sizing::TopDown ? eqsz.w : cl->rect.w);
					chalign(i, p, size2i{d, content.h});
					p.x += d + elemgap;
				}
				else
				{
					int d = (sz == Sizing::TopDown ? eqsz.h : cl->rect.h);
					chalign(i, p, size2i{content.w, d});
					p.y += d + elemgap;
				}
			}
		}

		size2i eqDivSize(int n) const
		{
			if(n == 0){ return {0,0}; }
			size2i szc = content.size();
			int g = (n-1)*elemgap;
			if(isHorizontal){ szc.w = (content.w - g) / n; }else{ szc.h = (content.h - g) / n; }
			return szc;
		}

		ListLayout(int elemgap_, bool isHorizontal_){ elemgap = elemgap_; isHorizontal = isHorizontal_; }

		size2i getContentSize(int n, ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) const override
		{
			auto b = computeBoundsOf(n, chsize);
			return size2i{isHorizontal ? b.W + (n-1)*elemgap : b.maxW, isHorizontal ? b.maxH : b.H + (n-1)*elemgap};
		};

		void update (int n, ChL chl, ChSz chsize, ChUp chupdate, std::optional<size2i> selfprefsz) override
		{
			auto ctsz = getContentSize(n, chl, chsize, chupdate, selfprefsz);
			boundChilds(n, ctsz, chl, chsize, chupdate);
		}

		void realign(int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign) override
		{
			alignContentAndRect(pos, outersz);
			alignChildAsList(n, chl, chalign/*[](Base* c, pos2i p, size2i s){ c->realign(p, s); }*/);
		}
	};
	
	struct Base
	{
		Layout* layout;

		Base(){ layout = new SingeElementLayout; }
		virtual size2i getPreferredSize() const{ return {0,0}; }
		virtual void updateContent(){}
		virtual void realign(pos2i pos, size2i outersz){}
		virtual void draw(SoftwareRenderer& sr){}
	};

	struct Leaf : Base
	{
		std::shared_ptr<ValueProxyBase> proxy;

		Leaf(){ layout = new SingeElementLayout; layout->gap = {8,8}; }

		void setProxy(std::shared_ptr<ValueProxyBase> p){ proxy = p; }
	
		size2i getPreferredSize() const override { return (proxy ? proxy->getSize() : size2i{0,0}) + 2*layout->gap; }

		void updateContent() override
		{
			if(proxy){ proxy->update(); }
			layout->update(0, [](int){ return nullptr; }, [](int){ return size2i{}; }, [](int){}, getPreferredSize());
			//layout.updateContent(getPreferredSize() - 2*layout.gap);
		}

		void realign(pos2i pos, size2i outersz) override
		{
			layout->realign(0, pos, outersz, [](int){ return nullptr; }, [](int){ return size2i{}; }, [](int, pos2i, size2i){});
			//layout.alignContentAndRect(pos, outersz);
		}

		void draw(SoftwareRenderer& sr) override
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(128,128,128));
			sr.rect(layout->content, color8(255,0,0));
			if(proxy){ proxy->draw(layout->content, sr); }
		}
	};

	struct Col2 : Base
	{
		std::array<Layout, 2> ls;
		std::array<Image2<unsigned char>, 2> imgs;
		Col2(){ layout = new Col2Layout; }

		size2i getPreferredSize() const override { return size2i{std::max(imgs[0].sz.w, imgs[1].sz.w), imgs[0].sz.h + imgs[1].sz.h} + 2*layout->gap; }

		void updateContent() override
		{
			layout->update(2, [&](int i){ return &(ls[i]); }, [&](int i){ return imgs[i].size(); }, [](int i){}, std::nullopt );
			//layout.updateContent(getPreferredSize()-2*layout.gap);
		}

		virtual void realign(pos2i pos, size2i outersz)
		{
			layout->realign(2, pos, outersz, [&](int i){ return &(ls[i]); }, [&](int i){ return imgs[i].size(); }, [](int, pos2i, size2i){} );
			//layout.alignContentAndRect(pos, outersz);
		}

		virtual void draw(SoftwareRenderer& sr)
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(128,128,128));
			sr.blend_grayscale_image(imgs[0], ls[0].rect.x, ls[0].rect.y, color8(255,255,0));
			sr.blend_grayscale_image(imgs[1], ls[1].rect.x, ls[1].rect.y, color8(0,255,255));
		}
	};

	struct ListBase : Base
	{
		ListLayout& getListLayout(){ return *((ListLayout*)layout); }
		ListBase(){ layout = new ListLayout(0, true); }

		virtual int         nElems() const { return 0; }
		virtual Base*       getElem(int i){ return nullptr; }
		virtual Base const* getElem(int i)const{ return nullptr; }
		virtual void        updateElem(int i){}
		virtual size2i      getElemSize(int i) const { return {0,0}; }
		virtual size2i      getPreferredSize() const override
		{
			int n = nElems();
			return layout->getContentSize(n, [this](int i){ return this->getElem(i)->layout; }, [&](int i){ return getElemSize(i); }, [](int i){}, std::nullopt) + 2*layout->gap;
			//auto b = computeBoundsOf(n, [&](int i){ return getElemSize(i); });
			//return size2i{isHorizontal ? b.W + (n-1)*elemgap : b.maxW, isHorizontal ? b.maxH : b.H + (n-1)*elemgap} + 2*layout.gap;
		};
	};

	struct List : ListBase
	{
		std::vector<Base*> childs;

		List()
		{
			((ListLayout*)layout)->elemgap = 4; ((ListLayout*)layout)->isHorizontal = false;
			layout->icva = VContentAlign::Center; layout->icha = HContentAlign::Fill;
		}

		int         nElems() const override { return (int)childs.size(); }
		Base*       getElem(int i) override { return childs[i]; }
		Base const* getElem(int i) const override { return childs[i]; }
		void        updateElem (int i) override { childs[i]->updateContent(); };
		size2i      getElemSize(int i) const override { return childs[i]->getPreferredSize(); }
		void        add( Base& x ){ childs.push_back(&x); }

		void updateContent()
		{
			for(int i=0; i<nElems(); ++i){ auto c = getElem(i); c->layout->cha = layout->icha; c->layout->cva = layout->icva; c->layout->sz = layout->sz; c->updateContent(); }
			layout->update(nElems(), [this](int i){ return childs[i]->layout; }, [this](int i){ return this->getElemSize(i); }, [this](int i){ this->childs[i]->updateContent(); }, std::nullopt);
			//boundChilds(*this, [](Base* c){ c->updateContent(); });
			//LayouterUpdate(layout, nElems(), [this](int i){ return this->getElem(i)->layout; }, [&](int i){ return getElemSize(i); }, [](int i){});
		}

		void realign(pos2i pos, size2i outersz)
		{
			layout->realign(nElems(), pos, outersz, [this](int i){ return childs[i]->layout; }, [this](int i){ return this->getElemSize(i); }, [this](int i, pos2i p, size2i s){ this->childs[i]->realign(p, s); });
			//layout.alignContentAndRect(pos, outersz);
			//alignChildAsList(*this, [](Base* c, pos2i p, size2i s){ c->realign(p, s); });
		}

		void draw(SoftwareRenderer& sr)
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(64,64,64));
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
		size2i getElemSize(int i) const override { return (proxy ? proxy->getElemSize(i) : size2i{0,0}) + 2*reference.layout->gap; }

		ListData()
		{
			((ListLayout*)layout)->elemgap = 2; ((ListLayout*)layout)->isHorizontal = false;
			reference.layout->gap = {1,1}; reference.layout->cha = HContentAlign::Fill; layout->icva = reference.layout->cva = VContentAlign::Center; reference.layout->sz = Sizing::BottomUp;
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
					c.layout = new SingeElementLayout(*(SingeElementLayout*)reference.layout);
					c.layout->sz = layout->sz;
					//is this needed?
					c.layout->updateContent(proxy->getElemSize(i));
					//c.layout->content = proxy->getElemSize(i);
					//if(c.layout->sz == Sizing::BottomUp){ /*c.layout->rect = center_shrink(c.layout->content, -c.layout->gap.w, -c.layout->gap.h);*/ }
				}
				layout->update(nElems(), [this](int i){ return this->chs[i].layout; }, [this](int i){ return this->getElemSize(i); }, [this](int i){ this->chs[i].layout->content = this->chs[i].layout->rect.size() - 2*this->chs[i].layout->gap; }, std::nullopt);
				//boundChilds(*this, [](Base* c){ c->layout.content = c->layout.rect.size() - 2*c->layout.gap; });
			}
		}

		void realign(pos2i pos, size2i outersz)override
		{
			layout->realign(nElems(), pos, outersz, [this](int i){ return this->chs[i].layout; }, [this](int i){ return this->getElemSize(i); }, [this](int i, pos2i p, size2i sz){ this->chs[i].layout->alignContentAndRect(p, sz); });
			//layout.alignContentAndRect(pos, outersz);
			//alignChildAsList(*this, [](Base* c, pos2i p, size2i s){ c->layout.alignContentAndRect(p, s); });
		}

		void draw(SoftwareRenderer& sr)override
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(64,64,64));
			if(proxy)
			{
				int n = nElems();
				for(int i=0; i<n; ++i){ proxy->drawElem(i, chs[i].layout->content, sr); }
			}
			//sr.rect(content, color8(255,0,255));
		}
	};
}