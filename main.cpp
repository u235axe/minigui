//#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <numeric>
#include "miniwindow.h"
#include "rendertext.h"

enum class HContentAlign { Left, Center, Right,  Fill };
enum class VContentAlign { Top,  Center, Bottom, Fill };
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

struct Base
{
	rect2i rect;
	rect2i content;
	size2i gap;
	HContentAlign ha;
	VContentAlign va;
	Sizing        sz;
	bool updateRq, realignRq;

	virtual size2i getPreferredSize() const{ return {0,0}; }
	virtual void updateContent(){}
	virtual void realign(pos2i pos, size2i outersz){}
	virtual void draw(SoftwareRenderer& sr){}
};

struct Text : Base
{
	PrerenderedText pt;

	Text(){ gap = {8,8}; ha = HContentAlign::Center; va = VContentAlign::Center; sz = Sizing::BottomUp; updateRq = true; realignRq = true; }

	void setText(utf8string const& str, StbFont const& font, float height)
	{
		pt = render_small_string_monospace(str, font, height);
		//pt.reduce_margins();
		updateRq = true;
	}

	virtual size2i getPreferredSize() const { return pt.text_align_box.size() + 2*gap; }

	virtual void updateContent()
	{
		if(sz == Sizing::BottomUp){ content = pt.text_align_box.size(); rect = center_shrink(content, -gap.w, -gap.h); }
		else                      { content = rect.size() - 2*gap; }
		updateRq = false; realignRq = true;
	}

	virtual void realign(pos2i pos, size2i outersz)
	{
		alignContentToOuter(rect, rect2i{pos.x, pos.y, outersz.w, outersz.h}, {0,0}, ha, va);
		alignContentToOuter(content, rect, gap, ha, va); realignRq = false;
	}

	virtual void draw(SoftwareRenderer& sr)
	{
		sr.framedrect(rect, color8(192,192,192), color8(128,128,128));
		sr.rect(content, color8(255,0,0));
		sr.prerendered_text_to_rect(pt, content, color8(255, 224, 0), HAlign::InnerLeft, VAlign::InnerTop);
	}
};

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

struct List : Base
{
	std::vector<Base*> childs;
	int elemgap;
	bool isHorizontal;

	int size() const { return (int)childs.size(); }

	List(){ gap = {8,8}; elemgap = 4; ha = HContentAlign::Center; va = VContentAlign::Center; sz = Sizing::TopDown; updateRq = true; realignRq = true; isHorizontal = false; }

	virtual size2i getPreferredSize() const
	{
		int n = size();
		auto b = computeBoundsOf(n, [&](int i){ childs[i]->updateContent(); return childs[i]->getPreferredSize(); });
		return size2i{isHorizontal ? b.W + (n-1)*elemgap : b.maxW, isHorizontal ? b.maxH : b.H + (n-1)*elemgap} + 2*gap;
	}

	size2i eqDivSize() const
	{
		int n = size();
		size2i szc = content.size();
		int g = (n-1)*elemgap;
		if(isHorizontal){ szc.w = (content.w - g) / n; }else{ szc.h = (content.h - g) / n; }
		return szc;
	}

	void updateContent()
	{
		for(auto& c : childs){ c->ha = HContentAlign::Fill; c->va = VContentAlign::Center; c->sz = sz; }
		if(sz == Sizing::BottomUp)
		{
			content = getPreferredSize() - 2*gap;
			rect = center_shrink(content, -gap.w, -gap.h);
		}
		else
		{//bound child's size from up by the available list entry space (computed from equal division of content) otherwise keeps the preferred size
			content = rect.size() - 2*gap;
			auto eqsz = eqDivSize();
			for(auto& c : childs)
			{
				c->rect = assignL(c->getPreferredSize(), eqsz);
				c->updateContent();
			}
		}
		updateRq = false; realignRq = true;
	}

	void realign(pos2i pos, size2i outersz)
	{
		alignContentToOuter(rect, rect2i{pos.x, pos.y, outersz.w, outersz.h}, {0,0}, ha, va);
		alignContentToOuter(content, rect, gap, ha, va);
		auto eqsz = eqDivSize();
		auto p = content.pos();
		for(auto& c : childs)
		{
			if(isHorizontal)
			{
				int d = (sz == Sizing::TopDown ? eqsz.w : c->rect.w);
				c->realign(p, size2i{d, content.h});
				p.x += d + elemgap;
			}
			else
			{
				int d = (sz == Sizing::TopDown ? eqsz.h : c->rect.h);
				c->realign(p, size2i{content.w, d});
				p.y += d + elemgap;
			}
		}
		realignRq = false;
	}

	void draw(SoftwareRenderer& sr)
	{
		sr.framedrect(rect, color8(192,192,192), color8(64,64,64));
		//sr.rect(rect, color8(192,192,192));
		for(auto& c : childs)
		{
			c->draw(sr);
		}
		//sr.rect(content, color8(255,0,255));
	}
};

struct App
{
	MainWindow wnd;
	Text       bQuit;
	List       list1;
	List       list2;
	List       list;
	StbFont    font;

	int enterApp()
	{
		font.init("DejaVuSansMono.ttf");
		bQuit.setText(utf8s("Quit"), font, 20);
		bQuit.sz = Sizing::BottomUp;

		list1.childs.push_back(new Text{});
		list1.childs.push_back(new Text{});
		list1.childs.push_back(new Text{});

		((Text*)list1.childs[0])->setText(utf8s(u8"[Á]"),   font, 20);
		((Text*)list1.childs[1])->setText(utf8s("vvvvvvv"), font, 20);
		((Text*)list1.childs[2])->setText(utf8s("."),       font, 20);

		list2.childs.push_back(new Text{});
		list2.childs.push_back(new Text{});
		list2.childs.push_back(new Text{});
		list2.childs.push_back(new Text{});

		((Text*)list2.childs[0])->setText(utf8s(u8"[É]"),   font, 20);
		((Text*)list2.childs[1])->setText(utf8s("wwwgwww"), font, 20);
		((Text*)list2.childs[2])->setText(utf8s("..."),     font, 20);
		((Text*)list2.childs[3])->setText(utf8s("123456789"),     font, 20);

		list.childs.push_back(&list1);
		list.childs.push_back(&list2);
		list.isHorizontal = true;
		list.elemgap = 10;
		//list.gap = {0,0};

		wnd.window.eventDriven = true;
		wnd.mouseHandler([&](Mouse const& m)
			{
				if(m.isLeftUp() && is_inside(bQuit.rect, m.pos))
				{
					wnd.quit();
				}
			});
		wnd.keyboardHandler([&](Keyboard const& k)
			{
				wnd.window.redraw();
			});
		wnd.resizeHandler( [&](int w, int h, bool b)
			{
				wnd.renderer.filledrect(0, 0, w, h, color8(0, 0, 0));
				bQuit.updateContent();
				bQuit.realign(pos2i{(int)(w * 0.05f), (int)(h * 0.05)}, {});
				
				list.rect = size2i{(int)(w * 0.5f), (int)(h * 0.5f)};
				list.updateContent();
				list.realign(pos2i{(int)(w * 0.5f), (int)(h * 0.5)}, {});
			} );
		wnd.idleHandler([&]
			{
			} );
		wnd.exitHandler([&]
			{
				wnd.quit();
			});
		wnd.renderHandler( [&](SoftwareRenderer& r)
			{
				wnd.renderer.filledrect(0, 0, wnd.width(), wnd.height(), color8(0, 0, 0));
				bQuit.draw(r);
				list.draw(r);
			});

		bool res = wnd.open(utf8s("GUI Test"), {4, 64}, {(int)(800), (int)(600)}, true, false, [&]{ return true; });
		return res ? 0 : -1;
	}
};

int main(int argc, char **argv)
{
	App app;
	return app.enterApp();
}
