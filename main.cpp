#ifdef _WIN32
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif
#include "ui2.h"
using namespace UI2;

struct App
{
	MainWindow wnd;
	StaticText staticText;
	ProxyValue bQuit;
	ProxyValue uiCounter;
	ListData   listd;

	TwoRows    tworows;
	TitleAndTwoCols tatc;

	int         counter;
	utf32string text1;
	std::vector<int> ints;

	utf32string texts[7];

	std::shared_ptr<List> list1;
	std::shared_ptr<List> list2;
	List       list;
	Style	   style;

	int enterApp()
	{
		style.font.init("DejaVuSansMono.ttf");
		style.height = 20;
		style.bg = color8(0, 0, 64);
		style.fg = color8(0, 192, 255);

		tworows.imgs[0] = render_small_string_monospace(utf8s(u8"W"), style.font, 48).reduce_margins().img;
		tworows.imgs[1] = render_small_string_monospace(utf8s(u8"2"), style.font, 16).reduce_margins().img;
		tworows.layout->gap = {1,1};
		tworows.layout->sz = Sizing::TopDown;

		tatc.imgs[0] = render_small_string_monospace(utf8s("This is the title"), style.font, 20).img;
		tatc.imgs[1] = render_small_string_monospace(utf8s("Subcontent 1"), style.font, 16).img;
		tatc.imgs[2] = render_small_string_monospace(utf8s("Subcontent 2"), style.font, 16).img;
		tatc.layout->gap = {2,2};

		staticText.font = &style.font;
		staticText.height = 26;
		staticText.text = utf8string(utf8s("My static text")).to_codepoints();

		text1   = utf8s("Quit").to_codepoints();
		counter = 0;

		bQuit.setProxy( view_value(text1, style) );
		uiCounter.setProxy( view_value(counter, style) );
		listd.setProxy( view_multi_value(ints, style) );

		bQuit.layout->sz     = Sizing::BottomUp;
		uiCounter.layout->sz = Sizing::BottomUp;

		listd.layout->sz     = Sizing::TopDown;
		listd.layout->cva    = VContentAlign::Top;
		listd.reference->cha = HContentAlign::Left;
		listd.reference->cva = VContentAlign::Top;
		listd.getListLayout().elemgap      = 2;
		listd.getListLayout().isHorizontal = false;

		texts[0] = utf8s(u8"[Á]").to_codepoints();
		texts[1] = utf8s(u8"vvvvvvv").to_codepoints();
		texts[2] = utf8s(u8".").to_codepoints();
		texts[3] = utf8s(u8"[É]").to_codepoints();
		texts[4] = utf8s(u8"wwwgwww").to_codepoints();
		texts[5] = utf8s(u8"...").to_codepoints();
		texts[6] = utf8s(u8"123456789").to_codepoints();

		list1 = std::make_shared<List>(2, false);
		//list1->layout->sz = Sizing::Top;
		list1->layout->gap = {4,4};
		list1->add( std::make_shared<ProxyValue>( view_value(texts[0], style) ) );
		list1->add( std::make_shared<ProxyValue>( view_value(texts[1], style) ) );
		list1->add( std::make_shared<ProxyValue>( view_value(texts[2], style) ) );

		list2 = std::make_shared<List>(2, false);
		list2->layout->gap = {4,4};
		//list2->layout->sz = Sizing::BottomUp;
		list2->add( std::make_shared<ProxyValue>( view_value(texts[3], style) ) );
		list2->add( std::make_shared<ProxyValue>( view_value(texts[4], style) ) );
		list2->add( std::make_shared<ProxyValue>( view_value(texts[5], style) ) );
		list2->add( std::make_shared<ProxyValue>( view_value(texts[6], style) ) );

		list.add(list1);
		list.add(list2);
		list.getListLayout().isHorizontal = true;
		list.getListLayout().elemgap = 10;
		list.layout->gap = {4,4};
		list.layout->sz = Sizing::TopDown;

		wnd.window.eventDriven = true;
		wnd.mouseHandler([&](Mouse const& m)
			{
				if(m.isLeftUp() && is_inside(bQuit.layout->rect, m.pos))
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

				uiCounter.updateContent();
				uiCounter.realign(pos2i{(int)(w * 0.125f), (int)(h * 0.225)}, {});
				uiCounter.draw(r);
				counter += 1;

				bQuit.updateContent();
				bQuit.realign(pos2i{(int)(w * 0.055f), (int)(h * 0.055)}, {});
				bQuit.draw(r);

				staticText.updateContent();
				staticText.realign(pos2i{(int)(w * 0.125f), (int)(h * 0.155)}, {});
				staticText.draw(r);

				tworows.layout->rect = size2i{(int)(w * 0.1f), (int)(h * 0.1f)};
				tworows.updateContent();
				tworows.realign(pos2i{(int)(w * 0.625f), (int)(h * 0.125)}, {});
				tworows.draw(r);

				tatc.updateContent();
				tatc.realign(pos2i{(int)(w * 0.625f), (int)(h * 0.825)}, {});
				tatc.draw(r);

				listd.layout->rect = size2i{(int)(w * 0.2f), (int)(h * 0.5f)};
				listd.updateContent();
				listd.realign(pos2i{(int)(w * 0.125f), (int)(h * 0.325)}, {});
				listd.draw(r);
				
				list.layout->rect = size2i{(int)(w * 0.5f), (int)(h * 0.5f)};
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
