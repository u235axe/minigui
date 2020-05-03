//#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
using byte = unsigned char;
#include "miniwindow.h"
#include "rendertext.h"

struct Button
{
	PrerenderedText pt;
	rect2i rect;

	void setText(utf8string const& str, StbFont const& font, float height)
	{
		pt = render_small_string_monospace(str, font, height);
		pt.reduce_margins();
		rect = pt.text_align_box.size();
	}

	void draw(SoftwareRenderer& sr)
	{
		auto bigger_rect = center_shrink(rect, -8, -8);
		sr.framedrect(bigger_rect, color8(192,192,192), color8(128,128,128));
		sr.prerendered_text(pt, rect.x, (int)(rect.y + pt.text_align_box.h * 0.8f), color8(255, 224, 0));
	}
};

struct App
{
	MainWindow wnd;
	Button     bQuit;
	StbFont    font;
	bool doQuit;

	int enterApp()
	{
		font.init("C:\\Users\\Admin\\Desktop\\minigui\\git\\DejaVuSansMono.ttf");
		bQuit.setText(utf8s("Quit"), font, 20);

		wnd.window.eventDriven = true;
		doQuit = false;

		wnd.mouseHandler([&](Mouse const& m)
			{
				if(m.isLeftUp() && is_inside(bQuit.rect, m.pos)){ doQuit = true; }
			});
		wnd.keyboardHandler([&](Keyboard const& k)
			{
				wnd.window.redraw();
			});
		wnd.resizeHandler( [&](int w, int h, bool b)
			{
				wnd.renderer.filledrect(0, 0, w, h, color8(0, 0, 0));
				bQuit.rect = pos2i{(int)(w * 0.05f), (int)(h * 0.05)};
			} );
		wnd.idleHandler([&]
			{
				if(doQuit){ wnd.quit(); }
			} );
		wnd.exitHandler([&]
			{
				wnd.quit();
				doQuit = true;
			});
		wnd.renderHandler( [&](SoftwareRenderer& r)
			{
				wnd.renderer.filledrect(0, 0, wnd.width(), wnd.height(), color8(0, 0, 0));
				bQuit.draw(r);
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