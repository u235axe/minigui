//#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "ui2.h"

using namespace UI2;

struct App
{
	MainWindow wnd;
	Leaf       bQuit;
	Leaf       uiCounter;
	ListData   listd;

	int         counter;
	utf32string text1;
	std::vector<int> ints;

	utf32string texts[7];

	List       list1;
	List       list2;
	List       list;
	Style	   style;

	int enterApp()
	{
		style.font.init("DejaVuSansMono.ttf");
		style.height = 20;
		style.bg = color8(0, 0, 64);
		style.fg = color8(0, 192, 255);

		text1   = utf8s("Quit").to_codepoints();
		counter = 0;

		bQuit.setProxy( view_value(text1, style) );
		uiCounter.setProxy( view_value(counter, style) );
		listd.setProxy( view_multi_value(ints, style) );

		bQuit.sz     = Sizing::BottomUp;
		uiCounter.sz = Sizing::BottomUp;
		listd.sz     = Sizing::BottomUp;
		listd.isHorizontal = false;

		texts[0] = utf8s(u8"[Á]").to_codepoints();
		texts[1] = utf8s(u8"vvvvvvv").to_codepoints();
		texts[2] = utf8s(u8".").to_codepoints();
		texts[3] = utf8s(u8"[É]").to_codepoints();
		texts[4] = utf8s(u8"wwwgwww").to_codepoints();
		texts[5] = utf8s(u8"...").to_codepoints();
		texts[6] = utf8s(u8"123456789").to_codepoints();

		list1.childs.push_back(new Leaf{});
		list1.childs.push_back(new Leaf{});
		list1.childs.push_back(new Leaf{});

		((Leaf*)list1.childs[0])->setProxy( view_value(texts[0], style) );
		((Leaf*)list1.childs[1])->setProxy( view_value(texts[1], style) );
		((Leaf*)list1.childs[2])->setProxy( view_value(texts[2], style) );

		list2.childs.push_back(new Leaf{});
		list2.childs.push_back(new Leaf{});
		list2.childs.push_back(new Leaf{});
		list2.childs.push_back(new Leaf{});

		((Leaf*)list2.childs[0])->setProxy( view_value(texts[3], style) );
		((Leaf*)list2.childs[1])->setProxy( view_value(texts[4], style) );
		((Leaf*)list2.childs[2])->setProxy( view_value(texts[5], style) );
		((Leaf*)list2.childs[3])->setProxy( view_value(texts[6], style) );

		list.childs.push_back(&list1);
		list.childs.push_back(&list2);
		list.isHorizontal = true;
		list.elemgap = 10;
		list.sz = Sizing::TopDown;
		//list.gap = {0,0};

		wnd.window.eventDriven = true;
		wnd.mouseHandler([&](Mouse const& m)
			{
				if(m.isLeftUp() && is_inside(bQuit.rect, m.pos))
				{
					wnd.quit();
				}
				if(m.isRightUp())
				{
					ints.push_back(rand());
				}
				wnd.window.redraw();
			});
		wnd.keyboardHandler([&](Keyboard const& k)
			{
				wnd.window.redraw();
			});
		wnd.resizeHandler( [&](int w, int h, bool b)
			{
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
				int w = wnd.width();
				int h = wnd.height();
				
				wnd.renderer.filledrect(0, 0, w, h, color8(0, 0, 0));

				bQuit.updateContent();
				bQuit.realign(pos2i{(int)(w * 0.125f), (int)(h * 0.125)}, {});

				uiCounter.updateContent();
				uiCounter.realign(pos2i{(int)(w * 0.125f), (int)(h * 0.225)}, {});

				listd.updateContent();
				listd.realign(pos2i{(int)(w * 0.125f), (int)(h * 0.325)}, {});

				bQuit.draw(r);
				uiCounter.draw(r);
				listd.draw(r);
				counter += 1;

				list.rect = size2i{(int)(w * 0.5f), (int)(h * 0.5f)};
				list.updateContent();
				list.realign(pos2i{(int)(w * 0.5f), (int)(h * 0.5)}, {});
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
