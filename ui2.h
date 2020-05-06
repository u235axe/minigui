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

		virtual size2i getContentSize(int n,                            ChSz chsize) const{ return {0,0}; };
		virtual void   update        (int n,                            ChL chl, ChSz chsize, ChUp chupdate){}
		virtual void   realign       (int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign){}
	};

	struct SingleElementLayout : Layout
	{
		SingleElementLayout(){}
		SingleElementLayout(SingleElementLayout const& cpy){ *this = cpy; }
		size2i getContentSize(int n, ChSz chsize) const override
		{
			return chsize(0) - 2*gap;
		};

		void update (int n, ChL chl, ChSz chsize, ChUp chupdate) override
		{
			updateContent(chsize(0) - 2*gap);
		}

		void realign(int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign) override
		{
			alignContentAndRect(pos, outersz);
		}
	};

	struct ListLayout : Layout
	{
		int elemgap;
		bool isHorizontal;

		//could be global?
		void boundChilds(int n, size2i contentSize, ChL chl, ChSz chsz, ChUp chup)
		{
			if(sz == Sizing::BottomUp)
			{
				content = contentSize;
				rect    = center_shrink(content, -gap.w, -gap.h);
				for(int i=0; i<n; ++i){ chup(i); }//this is not strictly necessary, but convenient, unifies implementation with TopDown handling for the end user
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

		size2i getContentSize(int n, ChSz chsize) const override
		{
			auto b = computeBoundsOf(n, chsize);
			return size2i{isHorizontal ? b.W + (n-1)*elemgap : b.maxW, isHorizontal ? b.maxH : b.H + (n-1)*elemgap};
		};

		void update (int n, ChL chl, ChSz chsize, ChUp chupdate) override
		{
			auto ctsz = getContentSize(n, chsize);
			boundChilds(n, ctsz, chl, chsize, chupdate);
		}

		void realign(int n, pos2i pos, size2i outersz, ChL chl, ChSz chsize, ChAl chalign) override
		{
			alignContentAndRect(pos, outersz);
			alignChildAsList(n, chl, chalign);
		}
	};
	
	struct Base
	{
		Layout* layout;

		Base(){ layout = new SingleElementLayout; }
		virtual size2i getPreferredSize() const{ return {0,0}; }
		virtual void updateContent(){}
		virtual void realign(pos2i pos, size2i outersz){}
		virtual void draw(SoftwareRenderer& sr){}
	};

	struct Leaf : Base
	{
		std::shared_ptr<ValueProxyBase> proxy;

		Leaf(){ layout = new SingleElementLayout; layout->gap = {8,8}; }

		void setProxy(std::shared_ptr<ValueProxyBase> p){ proxy = p; }
	
		size2i getPreferredSize() const override { return (proxy ? proxy->getSize() : size2i{0,0}) + 2*layout->gap; }

		void updateContent() override
		{
			if(proxy){ proxy->update(); }
			layout->update(0, [](int){ return nullptr; }, [&](int){ return getPreferredSize(); }, [](int){});
		}

		void realign(pos2i pos, size2i outersz) override
		{
			layout->realign(0, pos, outersz, [](int){ return nullptr; }, [](int){ return size2i{}; }, [](int, pos2i, size2i){});
		}

		void draw(SoftwareRenderer& sr) override
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(128,128,128));
			sr.rect(layout->content, color8(255,0,0));
			if(proxy){ proxy->draw(layout->content, sr); }
		}
	};

	struct TwoRows : Base
	{
		std::array<SingleElementLayout, 2> ls;
		std::array<Image2<unsigned char>, 2> imgs;
		TwoRows(){ layout = new ListLayout(1, false); }

		size2i getPreferredSize() const override { return ((ListLayout*)layout)->getContentSize(2, [&](int i){ return imgs[i].size() + 2*ls[i].gap; }); }

		void updateContent() override
		{
			layout->update(2, [&](int i){ return &(ls[i]); },
				              [&](int i){ return imgs[i].size() + 2*ls[i].gap; },
				              [&](int i){ ls[i].updateContent(imgs[i].size()); } );
		}

		void realign(pos2i pos, size2i outersz)
		{
			layout->realign(2, pos, outersz, [&](int i){ return &(ls[i]); },
				                             [&](int i){ return imgs[i].size(); },
				                             [&](int i, pos2i p, size2i s){ ls[i].alignContentAndRect(p, s); } );
		}

		void draw(SoftwareRenderer& sr)
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(128,128,128));
			sr.blend_grayscale_image(imgs[0], ls[0].content.x, ls[0].content.y, color8(255,255,0));
			sr.blend_grayscale_image(imgs[1], ls[1].content.x, ls[1].content.y, color8(0,255,255));
		}
	};

	struct TitleAndTwoCols : Base
	{
		std::array<SingleElementLayout, 2>     hls;
		std::array<std::shared_ptr<Layout>, 2> vls;
		std::array<Image2<unsigned char>, 3>   imgs;

		TitleAndTwoCols(){ layout = new ListLayout(2, false); vls[0] = std::make_shared<SingleElementLayout>(); vls[1] = std::make_shared<ListLayout>(2, true); }

		//size2i getPreferredSize() const override { return ((ListLayout*)layout)->getContentSize(2, [&](int i){ return imgs[i].size() + 2*ls[i].gap; }); }

		void updateContent() override
		{
			vls[1]->update(2, [&](int i){ return &(hls[i]); },
				              [&](int i){ return imgs[1+i].size() + 2*hls[i].gap; },
				              [&](int i){ hls[i].updateContent(imgs[1+i].size()); });

			layout->update(2, [&](int i){ return &*vls[i]; },
				              [&](int i){ return i == 0 ? imgs[i].size() + 2*vls[0]->gap : vls[1]->rect.size(); },
				              [&](int i){ vls[i]->updateContent(i == 0 ? imgs[0].size() : vls[1]->rect.size()); } );
		}

		void realign(pos2i pos, size2i outersz) override
		{
			layout->realign(2, pos, outersz, [&](int i){ return &(*vls[i]); },
				                             [&](int i){ return i == 0 ? imgs[i].size() + 2*vls[0]->gap : vls[1]->rect.size(); },
				                             [&](int i, pos2i p, size2i s)
				{
					vls[i]->realign(2, p, s, [&](int i){ return &(hls[i]); },
						                     [&](int i){ return imgs[1+i].size() + 2*hls[i].gap; },
						                     [&](int i, pos2i p, size2i s){ hls[i].alignContentAndRect(p, s); });
				} );
		}

		void draw(SoftwareRenderer& sr) override
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(128,128,128));
			sr.blend_grayscale_image(imgs[0], vls[0]->content.x, vls[0]->content.y, color8(255,0,0));
			sr.blend_grayscale_image(imgs[1], hls[0].content.x, hls[0].content.y, color8(0,255,0));
			sr.blend_grayscale_image(imgs[2], hls[1].content.x, hls[1].content.y, color8(0,0,255));
		}
	};

	struct ListBase : Base
	{
		ListLayout& getListLayout(){ return *((ListLayout*)layout); }
		ListBase(){ layout = new ListLayout(0, true); }

		virtual int         nElems() const { return 0; }
		virtual size2i      getPreferredSize() const override { return {0,0}; }
		virtual size2i      getElemSize(int i) const { return {0,0}; }
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
		size2i      getElemSize(int i) const override { return childs[i]->getPreferredSize(); }
		void        add( Base& x ){ childs.push_back(&x); }

		size2i getPreferredSize() const override { return layout->getContentSize(nElems(), [&](int i){ return getElemSize(i); }) + 2*layout->gap; }

		void updateContent() override
		{
			for(int i=0; i<nElems(); ++i){ auto& c = childs[i]; c->layout->cha = layout->icha; c->layout->cva = layout->icva; c->layout->sz = layout->sz; c->updateContent(); }
			layout->update(nElems(), [&](int i){ return childs[i]->layout; },
				                     [&](int i){ return getElemSize(i); },
				                     [&](int i){ childs[i]->updateContent(); });
		}

		void realign(pos2i pos, size2i outersz) override
		{
			layout->realign(nElems(), pos, outersz, [&](int i){ return childs[i]->layout; },
				                                    [&](int i){ return getElemSize(i); },
				                                    [&](int i, pos2i p, size2i s){ childs[i]->realign(p, s); });
		}

		void draw(SoftwareRenderer& sr) override
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(64,64,64));
			for(auto& c : childs){ c->draw(sr); }
			//sr.rect(content, color8(255,0,255));
		}
	};

	struct ListData : ListBase
	{
		std::shared_ptr<MultiValueProxyBase> proxy;
		std::shared_ptr<SingleElementLayout> reference;
		std::vector<std::shared_ptr<SingleElementLayout>> chs;

		int    nElems() const override { return proxy ? proxy->nElems() : 0; }
		size2i getElemSize(int i) const override { return (proxy ? proxy->getElemSize(i) : size2i{0,0}) + 2*reference->gap; }

		ListData()
		{
			((ListLayout*)layout)->elemgap = 2; ((ListLayout*)layout)->isHorizontal = false;
			reference = std::make_shared<SingleElementLayout>();
			reference->gap = {1,1};
			reference->sz = Sizing::BottomUp;
			layout->icha = reference->cha = HContentAlign::Fill;
			layout->icva = reference->cva = VContentAlign::Center;
		}

		void setProxy(std::shared_ptr<MultiValueProxyBase> p){ proxy = p; }

		void updateContent() override
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
					c = std::make_shared<SingleElementLayout>( *reference );
					c->sz = layout->sz;
					c->updateContent(proxy->getElemSize(i)); //is this needed?
				}
				layout->update(nElems(), [&](int i){ return &(*chs[i]); },
					                     [&](int i){ return getElemSize(i); },
					                     [&](int i){ chs[i]->updateContent(proxy->getElemSize(i)); });
			}
		}

		void realign(pos2i pos, size2i outersz) override
		{
			layout->realign(nElems(), pos, outersz, [&](int i){ return &(*chs[i]); },
				                                    [&](int i){ return getElemSize(i); },
				                                    [&](int i, pos2i p, size2i sz){ chs[i]->alignContentAndRect(p, sz); });
		}

		void draw(SoftwareRenderer& sr) override
		{
			sr.framedrect(layout->rect, color8(192,192,192), color8(64,64,64));
			if(proxy)
			{
				int n = nElems();
				for(int i=0; i<n; ++i){ proxy->drawElem(i, chs[i]->content, sr); }
			}
			//sr.rect(content, color8(255,0,255));
		}
	};
}