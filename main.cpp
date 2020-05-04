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

	virtual size2i getPreferredSize() const { return pt.text_align_box.size() + gap + gap; }

	virtual void updateContent()
	{
		if(sz == Sizing::BottomUp){ content = pt.text_align_box.size(); rect = center_shrink(content, -gap.w, -gap.h); }
		else                      { content = rect.size() - gap - gap; }
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

struct List : Base
{
	std::vector<Base*> childs;
	bool isHorizontal;

	List(){ gap = {8,8}; ha = HContentAlign::Center; va = VContentAlign::Center; sz = Sizing::TopDown; updateRq = true; realignRq = true; isHorizontal = false; }

	virtual size2i getPreferredSize() const
	{
		int W = 0, H = 0, maxW = 0, maxH = 0;
		int n = (int)childs.size();
		for(int i=0; i<n; ++i)
		{
			childs[i]->updateContent();
			auto chsz = childs[i]->getPreferredSize();
			maxW = std::max(maxW, chsz.w);
			maxH = std::max(maxH, chsz.h);
			W += chsz.w;
			H += chsz.h;
		}
		return size2i{isHorizontal ? W : maxW, isHorizontal ? maxH : H} + gap + gap;
	}

	void updateContent()
	{
		for(auto& c : childs){ c->ha = HContentAlign::Fill; c->va = VContentAlign::Center; c->sz = sz; }
		if(sz == Sizing::BottomUp)
		{
			content = getPreferredSize() - gap - gap;
			rect = center_shrink(content, -gap.w, -gap.h);
		}
		else
		{
			content = rect.size() - gap - gap;
			int n = (int)childs.size();
			if(isHorizontal)
			{
				int x = rect.x + gap.w;
				int w = content.w / n;
				for(int i=0; i<n; ++i)
				{
					auto pr = childs[i]->getPreferredSize();
					if(pr.h > content.h){ pr.h = content.h; }
					if(pr.w > w){ pr.w = w; }
					childs[i]->rect = pr;
					childs[i]->updateContent();
				}
			}
			else
			{
				int y = rect.y + gap.h;
				int h = content.h / n;
				for(int i=0; i<n; ++i)
				{
					auto pr = childs[i]->getPreferredSize();
					if(pr.w > content.w){ pr.w = content.w; }
					if(pr.h > h){ pr.h = h; }
					childs[i]->rect = pr;
					childs[i]->updateContent();
				}
			}
		}
		updateRq = false; realignRq = true;
	}

	void realign(pos2i pos, size2i outersz)
	{
		alignContentToOuter(rect, rect2i{pos.x, pos.y, outersz.w, outersz.h}, {0,0}, ha, va);
		alignContentToOuter(content, rect, gap, ha, va);
		int n = (int)childs.size();
		if(isHorizontal)
		{
			int x = content.x;
			int w0 = content.w / n;
			for(int i=0; i<n; ++i)
			{
				int w = sz == Sizing::TopDown ? w0 : childs[i]->rect.w;
				childs[i]->realign({x, content.y}, size2i{w, content.h});
				x += childs[i]->rect.w;
			}
		}
		else
		{
			int y = content.y;
			int h0 = content.h / n;
			for(int i=0; i<n; ++i)
			{
				int h = sz == Sizing::TopDown ? h0 : childs[i]->rect.h;
				childs[i]->realign({content.x, y}, size2i{content.w, h});
				y += childs[i]->rect.h;
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
				
				list.rect = size2i{(int)(w * 0.25f), (int)(h * 0.25)};
				list.updateContent();
				list.realign(pos2i{(int)(w * 0.25f), (int)(h * 0.25)}, {});
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
