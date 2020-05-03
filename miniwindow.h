#pragma once
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <functional>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WinUser.h>
#include <Windowsx.h>
#else
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xlocale.h>
//#include <X11/extensions/xf86vmode.h>
#endif
#include "utfstring.h"
#include "graphics_base.h"

enum class ButtonChange : bool {Up, Down};

struct Mouse
{
	enum Event{ Leave, Enter, Move, Scroll, Left, Middle, Right};
	pos2<int> pos;
	int  dz;
	Event event;
	ButtonChange change;
	bool left, middle, right;
	void ChangeLeft  (ButtonChange c){ change = c; event = Event::Left;   left   = (c == ButtonChange::Down); }
	void ChangeMiddle(ButtonChange c){ change = c; event = Event::Middle; middle = (c == ButtonChange::Down); }
	void ChangeRight (ButtonChange c){ change = c; event = Event::Right;  right  = (c == ButtonChange::Down); }
	bool isLeftDown  () const { return event == Event::Left   && change == ButtonChange::Down; }
	bool isLeftUp    () const { return event == Event::Left   && change == ButtonChange::Up;   }
	bool isMiddleDown() const { return event == Event::Middle && change == ButtonChange::Down; }
	bool isMiddleUp  () const { return event == Event::Middle && change == ButtonChange::Up;   }
	bool isRightDown () const { return event == Event::Right  && change == ButtonChange::Down; }
	bool isRightUp   () const { return event == Event::Right  && change == ButtonChange::Up;   }
	Mouse():pos{0,0}, dz{0}, event{Leave}, change{ButtonChange::Up}, left{false}, middle{false}, right{false}{} 
};

struct Keyboard
{
	enum Event{ LostFocus, GetFocus, Char, Backspace, Delete, Enter, ArrowLeft, ArrowRight, ArrowUp, ArrowDown };
	utf8string ch;
	Event event;
	ButtonChange change;
	bool backspace, del, enter, up, down, left, right;
	void ChangeBackspace (ButtonChange c){ change = c; event = Event::Backspace;  backspace = (c == ButtonChange::Down); }
	void ChangeDelete    (ButtonChange c){ change = c; event = Event::Delete;     del       = (c == ButtonChange::Down); }
	void ChangeEnter     (ButtonChange c){ change = c; event = Event::Enter;      enter     = (c == ButtonChange::Down); }
	void ChangeArrowUp   (ButtonChange c){ change = c; event = Event::ArrowUp;    up        = (c == ButtonChange::Down); }
	void ChangeArrowDown (ButtonChange c){ change = c; event = Event::ArrowDown;  down      = (c == ButtonChange::Down); }
	void ChangeArrowLeft (ButtonChange c){ change = c; event = Event::ArrowLeft;  left      = (c == ButtonChange::Down); }
	void ChangeArrowRight(ButtonChange c){ change = c; event = Event::ArrowRight; right     = (c == ButtonChange::Down); }
	bool isBackspaceDown  () const { return event == Event::Backspace  && change == ButtonChange::Down; }
	bool isBackspaceUp    () const { return event == Event::Backspace  && change == ButtonChange::Up;   }
	bool isDeleteDown     () const { return event == Event::Delete     && change == ButtonChange::Down; }
	bool isDeleteUp       () const { return event == Event::Delete     && change == ButtonChange::Up;   }
	bool isEnterDown      () const { return event == Event::Enter      && change == ButtonChange::Down; }
	bool isEnterUp        () const { return event == Event::Enter      && change == ButtonChange::Up;   }
	bool isLeftDown       () const { return event == Event::ArrowLeft  && change == ButtonChange::Down; }
	bool isLeftUp         () const { return event == Event::ArrowLeft  && change == ButtonChange::Up;   }
	bool isRightDown      () const { return event == Event::ArrowRight && change == ButtonChange::Down; }
	bool isRightUp        () const { return event == Event::ArrowRight && change == ButtonChange::Up;   }
	Keyboard():ch{}, event{Char}, change{ButtonChange::Up}, backspace{false}, del{false}, enter{false}, up{false}, down{false}, left{false}, right{false}{}
};

namespace MainWindowDetails
{
	struct ProcRelay
	{
		Mouse mouse;
		Keyboard keyboard;

		std::function<void(void)>            onRender;
		std::function<void(int, int, bool)>  onResize;
		std::function<void(void)>            onExit;
		std::function<void(Mouse    const&)> onMouseEvent;
		std::function<void(Keyboard const&)> onKeyboardEvent;

		ProcRelay():onRender{[]{}}, onResize{[](int, int, bool){}}, onExit{[]{}}, onMouseEvent{[](Mouse const&){}}, onKeyboardEvent{[](Keyboard const&){}}{}

		void mouse_trigger   (   Mouse::Event e){ mouse.event = e;      onMouseEvent(mouse); }
		void keyboard_trigger(Keyboard::Event e){ keyboard.event = e;   onKeyboardEvent(keyboard); }
		
		void mouse_xy    (pos2<int>   pos){ mouse.pos = pos;       mouse_trigger(Mouse::Move   ); }
		void mouse_z     (int          dz){ mouse.dz = dz;         mouse_trigger(Mouse::Scroll ); }
		void mouse_left  (ButtonChange  c){ mouse.ChangeLeft(c);   mouse_trigger(Mouse::Left   ); }
		void mouse_middle(ButtonChange  c){ mouse.ChangeMiddle(c); mouse_trigger(Mouse::Middle ); }
		void mouse_right (ButtonChange  c){ mouse.ChangeRight(c);  mouse_trigger(Mouse::Right  ); }

		void keyboard_char     (utf8string const&   ch){ keyboard.ch = ch;             keyboard_trigger(Keyboard::Char); }
		void keyboard_backspace(ButtonChange        c ){ keyboard.ChangeBackspace(c);  keyboard_trigger(Keyboard::Backspace); }
		void keyboard_delete   (ButtonChange        c ){ keyboard.ChangeDelete(c);     keyboard_trigger(Keyboard::Delete); }
		void keyboard_enter    (ButtonChange        c ){ keyboard.ChangeEnter(c);      keyboard_trigger(Keyboard::Enter); }
		void keyboard_left     (ButtonChange        c ){ keyboard.ChangeArrowLeft(c);  keyboard_trigger(Keyboard::ArrowLeft); }
		void keyboard_right    (ButtonChange        c ){ keyboard.ChangeArrowRight(c); keyboard_trigger(Keyboard::ArrowRight); }
		void keyboard_up       (ButtonChange        c ){ keyboard.ChangeArrowUp(c);    keyboard_trigger(Keyboard::ArrowUp); }
		void keyboard_down     (ButtonChange        c ){ keyboard.ChangeArrowDown(c);  keyboard_trigger(Keyboard::ArrowDown); }
	};
	/*inline*/ ProcRelay relay;

#ifdef _WIN32
	static LRESULT CALLBACK Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ButtonChange bc = ButtonChange::Up;
		switch(message)
		{
		case WM_MOUSEMOVE:   relay.mouse_xy(pos2<int>{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}); break;
		case WM_LBUTTONDOWN: relay.mouse_left(ButtonChange::Down);                       break;
		case WM_LBUTTONUP:   relay.mouse_left(ButtonChange::Up);                         break;
		case WM_MBUTTONDOWN: relay.mouse_middle(ButtonChange::Down);                     break;
		case WM_MBUTTONUP:   relay.mouse_middle(ButtonChange::Up);                       break;
		case WM_RBUTTONDOWN: relay.mouse_right(ButtonChange::Down);                      break;
		case WM_RBUTTONUP:   relay.mouse_right(ButtonChange::Up);                        break;
		case WM_MOUSEWHEEL:  relay.mouse_z(GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA);  break;

		case WM_CHAR:        relay.keyboard_char((wchar_t)wParam); break;
		case WM_KEYDOWN:
		bc = ButtonChange::Down;
		case WM_KEYUP:
		{
			switch(wParam)
			{
			case VK_BACK:   relay.keyboard_backspace(bc); break;
			case VK_RETURN: relay.keyboard_enter(bc);     break;
			case VK_DELETE: relay.keyboard_delete(bc);    break;
			case VK_LEFT:   relay.keyboard_left(bc);      break;
			case VK_RIGHT:  relay.keyboard_right(bc);     break;
			case VK_UP:     relay.keyboard_up(bc);      break;
			case VK_DOWN:   relay.keyboard_down(bc);     break;
			}
		}

		case WM_ERASEBKGND:  return 1;
		case WM_SIZE:        relay.onResize(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED); break;
		case WM_CLOSE:     relay.onExit();   break;
		case WM_PAINT:     relay.onRender(); break;
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}
#else
	static void Proc(Display* display, Window& handle, XIC& ic, XEvent e, size2<int> const& size, bool& isResizing)
	{
		ButtonChange bc = ButtonChange::Up;
		KeySym id;
		int  n;
		char buf[16];
		Status status;
		switch(e.type)
		{
		case MotionNotify: relay.mouse_xy(pos2<int>{e.xbutton.x, e.xbutton.y}); break;
		case EnterNotify:  break;
		case LeaveNotify:  break;
		case ButtonPress:
		bc = ButtonChange::Down;
		case ButtonRelease:
		{
			switch(e.xbutton.button)
			{
			case 1: relay.mouse_left(bc);   break;
			case 2: relay.mouse_middle(bc); break;
			case 3: relay.mouse_right(bc);  break;
			//case 4: relay.mouse_z(1);          break;
			//case 5: relay.mouse_z(-1);         break;
			}
			break;
		}
			
		case MappingNotify: XRefreshKeyboardMapping(&e.xmapping); break;
		case KeyPress:
		bc = ButtonChange::Down;
		case KeyRelease:
        {
            //printf( "Key %s : %x (%i)\n", bc == ButtonChange::Down ? "Down" : "Up", e.xkey.keycode, e.xkey.keycode );
			id = XLookupKeysym(&e.xkey, 0);
			//n = XLookupString( &e.xkey, &buf, 1, &id, NULL );
			if(bc == ButtonChange::Down)
			{
				
			}
			else
			{
				//
			}
			
			switch(id)
			{
			//case VK_BACK:   relay.keyboard_backspace(bc); break;
			case XK_Return:     relay.keyboard_enter(bc);      break;
			case XK_BackSpace:	relay.keyboard_backspace(bc);  break;
			case XK_Delete:	    relay.keyboard_delete(bc);     break;
			case XK_Left:	    relay.keyboard_left(bc);     break;
			case XK_Right:	    relay.keyboard_right(bc);     break;
			case XK_Shift_L:	break;
			case XK_Shift_R:	break;

			default:
			
			{
				if(bc == ButtonChange::Down /*&& id >= ' ' && id <= 127*/)
				{
					n = Xutf8LookupString(ic, (XKeyPressedEvent*)&e, buf, 16, &id, &status);
					//printf("%s (%c) (%i)\n", XKeysymToString(id), id, (int)id);
					//printf("buffer: %.*s\n", n, buf);
					//auto wstr = to_utf8_string(std::string(buf));
					//std::wcout << "after conversion: " << wstr << "\n";
					utf8string ustr; ustr.repr = std::string(buf);
					relay.keyboard_char(std::move(ustr));
				}
			}
			break;//relay.keyboard_char(XKeysymToString((id));    break;
			//case VK_DELETE: relay.keyboard_delete(bc);    break;
			//case VK_LEFT:   relay.keyboard_left(bc);      break;
			//case VK_RIGHT:  relay.keyboard_right(bc);     break;
			}
			break;
        }
		case ResizeRequest:
		{
			XResizeRequestEvent re = e.xresizerequest;
			//printf("RR\n");
			break;
		}
		case ConfigureRequest:
		{
			XConfigureRequestEvent re = e.xconfigurerequest;
			//printf("CR\n");
			break;
		}
		case ConfigureNotify:
		{
			XConfigureEvent re = e.xconfigure;
			if( size.w == re.width && size.h == re.height )
			{
				//printf("CN Move %i %i\n", re.x, re.y);
				//XMoveWindow(display, handle, 0, 0);
			}//move only
			else
			{
				isResizing = true;
				//printf("CN Resize %i %i\n", re.width, re.height);
				relay.onResize(re.width, re.height, true);
			}
			break;
		}
		//CM handled outside.
		case Expose:        /*printf("Expose ");*/ relay.onRender(); break;
		}
	}
#endif
}

struct PlatformWindowData
{
	pos2<int>  pos,  last_pos;
	size2<int> size, last_size;
	utf8string title;
#ifdef _WIN32
	WNDCLASS wc;
	DWORD type;
	DWORD style;
	HWND handle;
	HICON icon;
	bool eventDriven;
	PlatformWindowData():pos{0, 0}, last_pos{0, 0}, size{0, 0}, last_size{0, 0}, wc{0}, type{0}, style{0}, handle{nullptr}, icon{0}, eventDriven{true}{}

	bool open(utf8string const& title_, pos2<int> pos_, size2<int> size_, bool Decorated_, bool FullScreen_)
	{
		using namespace MainWindowDetails;
		wc = WNDCLASS{0};
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc   = Proc;
		wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(BLACK_BRUSH);
		wc.lpszClassName = L"MiniMainWindowClass";

		if( !RegisterClass(&wc) ){ return false; }

		type  = Decorated_ ? WS_OVERLAPPEDWINDOW : WS_POPUP;
		style = WS_CLIPCHILDREN  | WS_CLIPSIBLINGS;
		RECT rect0; rect0.left = pos_.x; rect0.right = pos_.x+size_.w; rect0.top = pos_.y; rect0.bottom = pos_.y+size_.h;
		if(!AdjustWindowRectEx(&rect0, style, FALSE, 0)){ return false; }

		pos.x  = rect0.left;
		pos.y  = rect0.top;
		size.w = rect0.right  - rect0.left;
		size.h = rect0.bottom - rect0.top;

		auto wtitle = title_.to_utf16();
		handle = CreateWindowEx(0, wc.lpszClassName, wtitle.c_str(), style | type, rect0.left, rect0.top, rect0.right - rect0.left, rect0.bottom - rect0.top, 0, 0, nullptr, (LPVOID)&relay);
		if( !handle ){ return false; }

		last_pos  = pos;
		last_size = size;
		title     = title_;
		return true;
	}

	bool rename(utf8string const& name)
	{
		auto wtitle = name.to_utf16();
		if(SetWindowTextW(handle, wtitle.c_str())){ title = name; return true; }
		return false;
	}

	bool setIcon( Image2<Color<unsigned char>> const& img )
	{
		HDC hdcScreen = GetDC(NULL);
		HDC hdcMem    = CreateCompatibleDC(hdcScreen);

		// Create the bitmap, and select it into the device context for drawing.
		HBITMAP hbmp    = CreateCompatibleBitmap(hdcScreen, img.w(), img.h());    
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmp);

		HDC     tmpdc  = CreateCompatibleDC(hdcScreen);
		HBITMAP bmp    = CreateBitmap(img.w(), img.h(), 1, 32, img.data.data());
		HGDIOBJ tmpbmp = SelectObject(tmpdc, bmp);
		/*auto b1 = */BitBlt(hdcMem, 0, 0, img.w(), img.h(), tmpdc, 0, 0, SRCCOPY);
		
		HBITMAP hbmpMask = CreateCompatibleBitmap(hdcScreen, img.w(), img.h());
		ICONINFO ii;
		ii.fIcon = TRUE;
		ii.hbmMask = hbmpMask;
		ii.hbmColor = hbmp;
		icon = CreateIconIndirect(&ii);
		DeleteObject(hbmpMask);

		// Clean-up.
		SelectObject(tmpdc, tmpbmp);
		DeleteObject(tmpbmp);
		ReleaseDC(NULL, tmpdc);
		SelectObject(hdcMem, hbmpOld);
		DeleteObject(hbmp);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcScreen);

		/*auto res = */SetClassLongPtr(handle, GCLP_HICON, (LONG_PTR)icon);
		return true;
	}

	template<typename F>
	void loop(F&& step)
	{
		MSG msg{0};
		DWORD last_time = 0;
		while( true ) 
		{
			if( eventDriven )
			{
				if( GetMessage(&msg, 0, 0, 0) < 0 ){ close(); }
				{
					TranslateMessage( &msg );
					DispatchMessage ( &msg );
					if(msg.message == WM_QUIT){ break; }
				}
				step();
				if(msg.time - last_time > 10)
				{
					redraw();
					last_time = msg.time;
				}
			}
			else
			{
				while( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0 )
				{
					TranslateMessage( &msg );
					DispatchMessage ( &msg );
					if(msg.message == WM_QUIT){ break; }
					
				}
				if(msg.message == WM_QUIT){ break; }
				step();
				//++last_time;
				//if(last_time > 2)
				{
					redraw();
					//last_time = 0;
				}
			}
		}
	}

	void redraw()   const { RedrawWindow(handle, 0, 0, RDW_ERASE|RDW_INVALIDATE); }
	void hide()     const { ShowWindow(handle, SW_HIDE); }
	void show()     const { ShowWindow(handle, SW_SHOW); SetFocus(handle); }
	void minimize()      { last_pos = pos; last_size = size; ShowWindow(handle, SW_MINIMIZE); }
	void maximize()      { last_pos = pos; last_size = size; ShowWindow(handle, SW_MAXIMIZE); }
	void restore() const { ShowWindow(handle, SW_RESTORE); }
	void quit()     const { PostQuitMessage(0); }
	bool close(){ int res = DestroyWindow(handle); handle = nullptr; eventDriven = true; return res != 0; }

	bool fullscreen(/*bool b*/)
	{
		/*bool res;
		if(b)
		{
			last_pos = pos;
			last_size = size;

			DEVMODE s{};
			s.dmSize = sizeof(s);
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &s);
			//s.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
			std::cout << "w x h = " << s.dmPelsWidth << " x " << s.dmPelsHeight << " depth: " << s.dmBitsPerPel << " Freq: " << s.dmDisplayFrequency << " Hz.\n";

			//SetWindowLongPtr(handle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
			SetWindowLongPtr(handle, GWL_STYLE,   WS_POPUP | WS_VISIBLE);
			SetWindowPos    (handle, HWND_TOPMOST, 0, 0, s.dmPelsWidth, s.dmPelsHeight, SWP_SHOWWINDOW);
			res = ChangeDisplaySettings(&s, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
			ShowWindow(handle, SW_MAXIMIZE);
			return res;
		}
		else
		{
			SetWindowLongPtr(handle, GWL_EXSTYLE, WS_EX_LEFT);
			SetWindowLongPtr(handle, GWL_STYLE, type | WS_VISIBLE);
			res = ChangeDisplaySettings(nullptr, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
			SetWindowPos(handle, HWND_NOTOPMOST, last_pos.x, last_pos.y, last_size.w, last_size.h, SWP_SHOWWINDOW);
			ShowWindow(handle, SW_RESTORE);
			return res;
		}*/
		return true;
	}
#else
	Display* display;
	Visual* visual;
	XIM     im;
    XIC     ic;
	int screen;
	unsigned int depth;
	Window handle;
	Atom AWM_DELETE_WINDOW, AWM_PROTOCOLS;
	bool eventDriven, needRedraw, isResizing, isQuit;
	PlatformWindowData():display{nullptr}, visual{nullptr}, screen{0}, eventDriven{true}, needRedraw{false}, isResizing{false}, isQuit{false}{}

	bool rename(utf8string const& name)
	{
		Atom awmname = XInternAtom(display, "_NET_WM_NAME", True);
		Atom autf8str = XInternAtom(display, "UTF8_STRING", True);
		if(awmname == None){ printf("_NET_WM_NAME not found. Window title was not set.\n"); }
		else
		{
			if(autf8str == None){ printf("UTF8_STRING not found. Window title was not set.\n"); }
			else
			{
				XChangeProperty(display, handle, awmname, autf8str, 8, PropModeReplace, (const unsigned char*)name.repr.c_str(), name.repr.size());
				title = name;
				return true;
			}
		}
		return false;
	}

	bool open(utf8string const& title_, pos2<int> pos_, size2<int> size_, bool Decorated_, bool FullScreen_)
	{
		printf("Creating window\n");

		display = XOpenDisplay(0);
		if(!display){ printf("Cannot open display\n"); return false; }
		screen = DefaultScreen(display);
		visual = DefaultVisual(display, screen);
		depth = DefaultDepth(display, screen);
		XVisualInfo vi;
		if( XMatchVisualInfo(display, screen, 32, TrueColor, &vi) != 0 )
		{
			visual = vi.visual;
			depth = vi.depth;
			printf("XMatchVisual success!\n");
		}
		else
		{
			printf("Visual not found! Default is used.\n");
		}

		if( visual->c_class != TrueColor )
		{
			printf("No TrueColor...\n");
		}

		if( setlocale(LC_ALL, "") == NULL)
		{ 
			printf("setlocale(LC_ALL) failed.\n");
		}

		if( !XSupportsLocale() )
		{
			printf("X does not support locale.\n");
		}

		//https://stackoverflow.com/questions/18246848/get-utf-8-input-with-x11-display%EF%BC%89
		if( XSetLocaleModifiers("@im=none") == NULL)
		{
			printf("XSetLocaleModifiers(\"@im=none\") failed.\n");
		}

		XSetWindowAttributes wa{0};
		wa.colormap = XCreateColormap(display, DefaultRootWindow(display), visual, AllocNone);
		//wa.background_pixel = 0;
		wa.border_pixel = 0;
		wa.backing_store = Always;
		wa.background_pixmap = None;
		//wa.override_redirect = !Decorated_ ? True : False;
		unsigned long mask = /*CWBackPixel | */CWBackPixmap | CWColormap | CWBorderPixel | CWBackingStore;// | CWOverrideRedirect;
		printf("Depth = %i, x = %i, y = %i\n", depth, pos_.x, pos_.y);
		handle = XCreateWindow(display, DefaultRootWindow(display), pos_.x, pos_.y, size_.w, size_.h, 0, depth, InputOutput, visual, mask, &wa);
		printf("handle = %ld\n", handle);
		isQuit = false;
		XSelectInput(display, handle, ExposureMask | StructureNotifyMask | SubstructureNotifyMask //| SubstructureRedirectMask // | */ResizeRedirectMask
			| KeyPressMask | KeyReleaseMask
			| ButtonPressMask | PointerMotionMask| LeaveWindowMask| EnterWindowMask| ButtonReleaseMask );
		
		rename(title_);
		last_pos = pos = pos_;
		last_size = size = size_;

		AWM_PROTOCOLS     = XInternAtom(display, "WM_PROTOCOLS", False);
		AWM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, handle, &AWM_DELETE_WINDOW, 1);

		if(!Decorated_)
		{
			struct Hints
			{
				unsigned long   flags;       //Functions 1, Decorations 2, InputMode 4, Status 8
				unsigned long   functions;   //None 0, All 1, Resize 2, Move 4, Minimize 8, Maximize 16, Close 32
				unsigned long   decorations; //None 0, All 1, Border 2, ResizeH 4, Title 8, Menu 16, Minimize 32, Maximize 64
				long            inputMode;   //Modeless = 0, PrimaryApplicationModal = 1, SystemModal = 2, FullApplicationModal = 3
				unsigned long   status;      //TearoffWindow 1
			};
			Hints   hints; hints.flags = 2; hints.decorations = 0;
			Atom    prop = XInternAtom(display,"_MOTIF_WM_HINTS",True);
			XChangeProperty(display, handle, prop, prop, 32, PropModeReplace, (unsigned char*)&hints, 5);
		}

		//UTF-8 support:
		im = XOpenIM(display, NULL, NULL, NULL);
		if(im == NULL)
		{
			printf("Could not open input method\n");
			return false;
		}
		
		XIMStyles* styles;
		char*      fail = XGetIMValues(im, XNQueryInputStyle, &styles, NULL);
		
		if(fail != NULL)
		{
			printf("XIM Can't get styles\n");
			return false;
		}

		for (int i = 0; i<styles->count_styles; i++)
		{
			printf("style %d\n", (int)styles->supported_styles[i]);
		}

		ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, handle, NULL);
		if(ic == NULL)
		{
			printf("Could not open IC\n");
			return false;
		}

		XSetICFocus(ic);
		return true;
	}

	template<typename F>
	void loop(F&& step)
	{
		auto t0 = std::chrono::high_resolution_clock::now();
		auto t1 = t0, trsz = t0;
		printf("Entering event loop\n");
		long nredraw = 0, nredrawproc = 0;

		auto sendExposeEvent = [&]()
		{
			XExposeEvent xee{0};
			xee.type = Expose;
			xee.send_event = True;
			xee.display = display;
			xee.window = handle;
			XSendEvent(display, handle, False, NoEventMask, (XEvent*)&xee);
		};

		//Reposition the window, because some WMs move the window initially despite the x, y, set in create window.
		XMoveResizeWindow(display, handle, last_pos.x, last_pos.y, last_size.w, last_size.h);
		while(true)
		{
			XEvent e;
			if(eventDriven)
			{
				XNextEvent(display, &e);
				if(XFilterEvent(&e, handle)){ continue; }
				if(e.type == ClientMessage && e.xclient.message_type == AWM_PROTOCOLS && e.xclient.data.l[0] == AWM_DELETE_WINDOW){ isQuit = true; MainWindowDetails::relay.onExit(); break; }
				else{ MainWindowDetails::Proc(display, handle, ic, e, size, isResizing); }
				//step();
				//printf("while 1\n");
				needRedraw = true;
			}
			else
			{
				auto res = True;
				//while(res == True && !isQuit)
				{
					while(XEventsQueued(display, QueuedAlready) > 0)
					{
						XNextEvent(display, &e);
						if(XFilterEvent(&e, handle)){ continue; }
						if(e.type == ClientMessage)
						{
							if(e.xclient.send_event){ /*printf("[%zi] Sent CM ", e.xclient.serial);*/ }
							else{ printf("[%zi] Not sent CM ", e.xclient.serial); }

							if(e.xclient.message_type == AWM_PROTOCOLS)
							{
								printf("WM_PROTOCOLS ");
								if(e.xclient.data.l[0] == AWM_DELETE_WINDOW){ printf("AWM_DELETE_WINDOW\n"); isQuit = true; MainWindowDetails::relay.onExit(); break; }
								else{ printf("Unknown protocol message %zi\n", e.xclient.data.l[0]); }
							}
							else if(e.xclient.message_type == 424242){ nredrawproc = e.xclient.data.l[0]; /*printf("Redraw processed: %zi\n", nredrawproc);*/ }
							else{ printf("Other Atom %zi ", e.xclient.message_type); }
						}
						else
						{
							bool wasResize = false;
							MainWindowDetails::Proc(display, handle, ic, e, size, wasResize);
							if(wasResize){ isResizing = true; trsz = std::chrono::high_resolution_clock::now(); }
						}
					}
					needRedraw = true;
				}
			}

			if(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-trsz).count()/1000.0 > 300.0)
			{
				//printf("isResizing = false\n");
				isResizing = false;
				needRedraw = true;
			}

			if(needRedraw)
			{
				double limit = (isResizing) ? 250.0 : 100.0;
				t1 = std::chrono::high_resolution_clock::now();
				auto dt = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()/1000.0;
				if(dt > limit)
				{
					if(eventDriven){ /*printf("limited redraw, expose event, dt = %f\n", dt);*/ sendExposeEvent();/*XClearArea(display, handle, 0, 0, 1, 1, true);*/ }
					else if(!eventDriven && (nredraw == nredrawproc))
					{
						step();
						sendExposeEvent();//XClearArea(display, handle, 0, 0, 1, 1, true);
						nredraw += 1;
						XEvent ev{0};
						ev.type = ClientMessage;
						ev.xclient.format = 32;
						ev.xclient.window = handle;
						ev.xclient.message_type = 424242;
						ev.xclient.data.l[0] = nredraw;
						XSendEvent(display, handle, False, NoEventMask, &ev);
						XSync(display, False);
						//XFlush(display);
					}
					t0 = t1;
					needRedraw = false;
				}
			}
			//
			//
			if(isQuit){ break; }
		}
	}

	/*void print_last() const
	{
		printf("lx= %i, ly= %i, lw= %i, lh= %i\n", last_pos.x, last_pos.y, last_size.w, last_size.h);
		printf("x= %i, y= %i, w= %i, h= %i\n", pos.x, pos.y, size.w, size.h);
	}*/

	void redraw() { /*printf("Redraw req\n");*/ needRedraw = true; XSync(display, False); }
	void show() const { printf("show\n"); XMapRaised(display, handle); }
	void hide() const { XUnmapWindow(display, handle); }
	void minimize() { last_pos = pos; last_size = size; XIconifyWindow(display, handle, 0); }
	void maximize()
	{
		last_pos = pos; last_size = size;

		XEvent xev{};
		Atom wm_state  =  XInternAtom(display, "_NET_WM_STATE", False);
		Atom max_horz  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		Atom max_vert  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

		xev.type = ClientMessage;
		xev.xclient.window = handle;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;//Add
		xev.xclient.data.l[1] = max_horz;
		xev.xclient.data.l[2] = max_vert;
		xev.xclient.data.l[3] = 0;
		XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);
		XSync(display, False);
	}

	void restore()
	{
		//switch back:
		/*int n;
		XF86VidModeModeInfo** modes;
		XF86VidModeGetAllModeLines(display, screen, &n, &modes);
		XF86VidModeSwitchToMode(display, screen, modes[0]);
		XF86VidModeSetViewPort(display, screen, 0, 0);
		XFree(modes);*/

		/*XSetWindowAttributes attributes;
		attributes.override_redirect = False;
		XChangeWindowAttributes(display, handle, CWOverrideRedirect, &attributes);*/

		//XSync(display, False);

		/*{
			XEvent xev{};
			Atom wm_state  =  XInternAtom(display, "_NET_WM_STATE", False);
			Atom max_horz  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
			Atom max_vert  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

			xev.type = ClientMessage;
			xev.xclient.window = handle;
			xev.xclient.message_type = wm_state;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 0;//Remove
			xev.xclient.data.l[1] = max_horz;
			xev.xclient.data.l[2] = max_vert;
			xev.xclient.data.l[3] = 1;
			XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);
		}*/

		{
			XEvent xev{};
			Atom wm_state      = XInternAtom(display, "_NET_WM_STATE", False);
			Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

			xev.type = ClientMessage;
			xev.xclient.window = handle;
			xev.xclient.message_type = wm_state;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 0;//Remove
			xev.xclient.data.l[1] = wm_fullscreen;
			xev.xclient.data.l[2] = 0;
			xev.xclient.data.l[3] = 1;
			XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);
		}
		//XSync(display, False);
		
		XMoveResizeWindow(display, handle, last_pos.x, last_pos.y, last_size.w, last_size.h);
		pos = last_pos;
		size = last_size;
		//XRaiseWindow(display, handle);
		XSync(display, False);
	}

	bool setIcon( Image2<Color8> const& img )
	{
		Atom net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
		Atom cardinal = XInternAtom(display, "CARDINAL", False);
		size_t length = 2 + img.w() * img.h();
		std::vector<unsigned long> data; data.resize(length);
		data[0] = img.w();
		data[1] = img.h();
		for(int i=0; i<img.w()*img.h(); ++i)
		{
			data[2+i] = packed_color(img.data[i]);
		}
		XChangeProperty(display, handle, net_wm_icon, cardinal, 32, PropModeReplace, (const unsigned char*)data.data(), length);
		XSync(display, False);
		return true;
	}

	void quit() const
	{
		XEvent e;
		e.xclient.type = ClientMessage;
		e.xclient.window = handle;
		e.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
		e.xclient.format = 32;
		e.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
		e.xclient.data.l[1] = CurrentTime;
		XSendEvent(display, handle, False, NoEventMask, &e);
		XSync(display, False);
	}
	bool close(){ XDestroyWindow(display, handle); XCloseDisplay(display); eventDriven = true; return true; }

	void fullscreen()
	{
		last_pos = pos; last_size = size;
		//maximize();
		/*Atom wm_state      = XInternAtom(display, "_NET_WM_STATE", False );
		Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False );
		XChangeProperty(display, handle, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);*/


		XEvent xev{};
		Atom wm_state      = XInternAtom(display, "_NET_WM_STATE", False);
		Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

		xev.type = ClientMessage;
		xev.xclient.window = handle;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;//Add
		xev.xclient.data.l[1] = wm_fullscreen;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 1;
		XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);


		/*XSetWindowAttributes attributes;
		attributes.override_redirect = True;
		XChangeWindowAttributes(display, handle, CWOverrideRedirect, &attributes);*/

		XSync(display, False);

		/*int n;
		XF86VidModeModeInfo** modes;
		XF86VidModeModeInfo* fullscreenMode;

		auto screenWidth  = XWidthOfScreen(XScreenOfDisplay(display, screen));
		auto screenHeight = XHeightOfScreen(XScreenOfDisplay(display, screen));

		//switch to full screen mode:
		XF86VidModeGetAllModeLines (display, screen, &n, &modes);
		for (int i = 0; i < n; ++i)
		{
			if ((modes[i]->hdisplay == screenWidth) &&
				(modes[i]->vdisplay == screenHeight))
			{
				fullscreenMode = modes[i];
			}
		}

		XF86VidModeSwitchToMode(display, screen, fullscreenMode);
		XF86VidModeSetViewPort(display, screen, 0, 0);
		auto displayWidth = fullscreenMode->hdisplay;
		auto displayHeight = fullscreenMode->vdisplay;
		XFree(modes);*/

		/* Warp mouse pointer at center of the screen */
		//XWarpPointer(display, None, handle, 0, 0, 0, 0, displayWidth / 2, displayHeight / 2);
		//XMapRaised(display, win->win);
		//XGrabKeyboard(display, handle, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		//XGrabPointer(display, handle, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, handle, None, CurrentTime);
		//XSync(display, False);
	}
#endif
};

struct SoftwareRenderer
{
	Image2<Color8> backbuffer;

	void init  (int w, int h){ backbuffer.resize({w, h}); }
	void resize(int w, int h){ /*printf("Renderer resize %i %i\n", w, h);*/ backbuffer.resize({w, h}); }
	void close(){}

	void setpixel(int x, int y, Color8 c)
	{
		if(clamp(x, 0, backbuffer.w()-1) == x && clamp(y, 0, backbuffer.h()-1) == y){ backbuffer(x, y) = c; }
	}

	Color8 getpixel(int x, int y)
	{
		if(clamp(x, 0, backbuffer.w()-1) == x && clamp(y, 0, backbuffer.h()-1) == y){ return backbuffer(x, y); }
		else{ return Color8{0, 0, 0, 0}; }
	}

	template<typename F>
	void forall_pixels(F&& f)
	{
		const int w = backbuffer.w();
		for(int y=0; y<backbuffer.h(); ++y)
		{
			int i = y * w;
			for(int x=0; x<w; ++x, ++i)
			{
				backbuffer.data[i] = f(x, y, backbuffer.data[i]);
			}
		}
	}

	template<typename T, typename F>
	void lineplot(int x, int y, int w, int h, T xmin, T xmax, Color8 col, F&& f)
	{
		using R = decltype(f(xmin));
		auto n = std::abs(w);
		if(n == 0){ return; }
		std::vector<R> tmp(n);

		R ymin = std::numeric_limits<R>::max();
		R ymax = std::numeric_limits<R>::min();
		for(int i=0; i<n; ++i)
		{
			T xc = ((T)i/(T)n)*(xmax - xmin) + xmin;
			auto yc = f(xc);
			if( is_finite(yc) )
			{
				if(yc < ymin){ ymin = yc; }
				if(yc > ymax){ ymax = yc; }
			}
			tmp[i] = yc;
		}

		auto yl = tmp[0];
		auto iyl = y + h - (yl - ymin) / (ymax - ymin) * h;
		for(int i=1; i<n; ++i)
		{
			auto yc = tmp[i];
			if(is_finite(yl) && is_finite(yc))
			{
				auto iyc = y + h - (yc - ymin) / (ymax - ymin) * h;
				line(x+i-1, (int)iyl, x+i, (int)iyc, [&](auto){ return col; });
				iyl = iyc;
				yl = yc;
			}
		}
	}

	template<typename IT>
	void barplot(int x, int y, int w, int h, IT begin, IT end, Color8 col)
	{
		using R = std::remove_reference_t<decltype(*begin)>;
		auto n = std::distance(begin, end);
		if(n == 0){ return; }
		
		R ymin = std::numeric_limits<R>::max();
		R ymax = std::numeric_limits<R>::min();
		for(auto i = begin; i!=end; ++i)
		{
			auto yc = *i;
			if( is_finite(yc) )
			{
				if(yc < ymin){ ymin = yc; }
				if(yc > ymax){ ymax = yc; }
			}
		}

		auto dx = (float)w / (float)n;
		auto X = x;
		auto z = y + h - ((float)0 - (float)ymin) / ((float)ymax - (float)ymin) * h;
		for(auto it = begin; it!=end; ++it)
		{
			auto yc = *it;
			auto hc = y + h - ((float)yc - (float)ymin) / ((float)ymax - (float)ymin) * h;
			if(is_finite(yc))
			{
				if(yc > 0)
				{
					filledrect(X, (int)hc, (int)dx, (int)(z - hc), col);
				}
				else
				{
					filledrect(X, (int)z, (int)dx, (int)(hc - z), col);
				}
				
			}
			X = (int)(X + dx);
		}
	}

	template<typename F>
	void plot_by_index(int x, int y, int w, int h, F&& f)
	{
		if(w <= 0 || h <= 0){ return; }
		auto ymin = clamp(y,   0, backbuffer.h()-1);
		auto ymax = clamp(y+h, 0, backbuffer.h()-1);
		auto xmin = clamp(x,   0, backbuffer.w()-1);
		auto xmax = clamp(x+w, 0, backbuffer.w()-1);
		for(int j=ymin; j<ymax; ++j)
		{
			if(j - ymin >= h){ break; }
			int k = j * backbuffer.w() + xmin;
			for(int i=xmin; i<xmax; ++i, ++k)
			{
				if(i - xmin >= w){ break; }
				backbuffer.data[k] = f(i-x , j-y, backbuffer.data[k]);
			}
		}
	}
	template<typename T, typename F>
	void plot_by_index(rect2<T> r, F&& f){ plot_by_index(r.x, r.y, r.w, r.h, std::forward<F>(f)); }

	void rect(int x, int y, int w, int h, Color8 col)
	{
		if(w <= 0 || h <= 0){ return; }
		line(x,   y,   x+w, y,   [&](auto){ return col; });
		line(x+w, y,   x+w, y+h, [&](auto){ return col; });
		line(x+w, y+h, x,   y+h, [&](auto){ return col; });
		line(x,   y+h, x,   y,   [&](auto){ return col; });
	}
	void rect(rect2<int> r, Color8 col){ rect(r.x, r.y, r.w, r.h, col); }

	void filledrect(int x, int y, int w, int h, Color8 col)
	{
		if(w <= 0 || h <= 0){ return; }
		auto ymin = clamp(y,   0, backbuffer.h()-1);
		auto ymax = clamp(y+h, 0, backbuffer.h()-1);
		auto xmin = clamp(x,   0, backbuffer.w()-1);
		auto xmax = clamp(x+w, 0, backbuffer.w()-1);
		for(int j=ymin; j<=ymax; ++j)
		{
			int k = j * backbuffer.w() + xmin;
			for(int i=xmin; i<=xmax; ++i, ++k)
			{
				backbuffer.data[k] = col;
			}
		}
	}
	void filledrect(rect2<int> r, Color8 col){ filledrect(r.x, r.y, r.w, r.h, col); }

	void framedrect(rect2<int> r, Color8 border, Color8 fill)
	{
		filledrect(r.x, r.y, r.w, r.h, fill);
		rect      (r.x, r.y, r.w, r.h, border);
	}

	void framedrect(int x, int y, int w, int h, Color8 border, Color8 fill)
	{
		filledrect(x, y, w, h, fill);
		rect      (x, y, w, h, border);
	}

	template<typename F>
	void line(int x0, int y0, int x1, int y1, F&& f)
	{
		int dx = abs(x1-x0);
		int sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1-y0);
		int sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;
		int e2 = 0;
		for (;;){
			setpixel(x0, y0, f(getpixel(x0, y0)));
			e2 = 2*err;
			if (e2 >= dy) {
				if (x0 == x1) break;
				err += dy; x0 += sx;
			}
			if (e2 <= dx) {
				if (y0 == y1) break;
				err += dx; y0 += sy;
			}
		}
	}

	template<typename I, typename F>
	void hline(int x0, int x1, int y, I&& i, F&& f)
	{
		if(y < 0 || y >= backbuffer.h()){ return; }
		x0 = std::max(0, x0); x0 = std::min(x0, backbuffer.w()-1);
		x1 = std::max(0, x1); x1 = std::min(x1, backbuffer.w()-1);
		for(int x = std::min(x0, x1); x<=std::max(x0, x1); ++x){ if(i(x, y)){ backbuffer(x, y) = f(backbuffer(x, y)); } }
	}

	template<typename I, typename F>
	void vline(int x, int y0, int y1, I&& i, F&& f)
	{
		if(x < 0 || x >= backbuffer.w()){ return; }
		y0 = std::max(0, y0); y0 = std::min(y0, backbuffer.h()-1);
		y1 = std::max(0, y1); y1 = std::min(y1, backbuffer.h()-1);
		for(int y = std::min(y0, y1); y<=std::max(y0, y1); ++y){ if(i(x, y)){ backbuffer(x, y) = f(backbuffer(x, y)); } }
	}

	void line (int x0, int y0, int x1, int y1, Color8 c){ line(x0, y0, x1, y1, [=](auto){ return c; }); } 
	void hline(int x0, int x1, int y,  Color8 c){ hline(x0, x1, y, [](auto, auto){ return true; }, [=](auto){ return c; }); } 
	void vline(int x,  int y0, int y1, Color8 c){ vline(x, y0, y1, [](auto, auto){ return true; }, [=](auto){ return c; }); } 

	void blend_grayscale_image(Image2<unsigned char> const& img, int x, int y, Color8 fg)
	{
		rect2i rct; rct = img.rect(); rct = pos2i{x, y};
		plot_by_index(rct, [&](auto x, auto y, Color8 c0){ return blend8(c0, img(x, y), fg); } );
	}

	void copy_image(Image2<Color8> const& img, int x, int y)
	{
		rect2i rct; rct = img.rect(); rct = pos2i{x, y};
		plot_by_index(rct, [&](auto x, auto y, Color8){ return img(x, y); } );
	}

	size2<int> prerendered_text(PrerenderedText const& pt, int x, int baseline, Color8 fg, HAlign ha = HAlign::InnerLeft)
	{
		auto dx = -pt.text_align_box.x;
		if     (ha == HAlign::HCenter)    { dx -= pt.text_align_box.w/2; }
		else if(ha == HAlign::InnerRight ){ dx -= pt.text_align_box.w; } 
		auto rct = pt.img.rect(); rct = pos2<int>{x + dx, baseline - pt.baseline};
		plot_by_index(rct, [&](auto x, auto y, Color8 c0){ return blend8(c0, pt.img(x, y), fg); } );
		return {x + rct.w, baseline + pt.dh};
	}

	template<typename T, typename F>
	void triangle(T x0, T y0, T x1, T y1, T x2, T y2, F&& f)
	{
		auto isInside = [](auto X0, auto Y0, auto X1, auto Y1, auto X2, auto Y2/*, auto b0, auto b1, auto b2*/)
		{
			auto edgeFunction = [](auto px0, auto py0, auto px1, auto py1, auto px2, auto py2){ return (px2 - px0) * (py1 - py0) - (py2 - py0) * (px1 - px0); };
			return [=](T x, T y)
			{
				const float c = 0.0f;
				const float d = 0.0f;
				auto w0 = edgeFunction(X0+d, Y0+d, X1+d, Y1+d, x+c, y+c);//+b0; 
				auto w1 = edgeFunction(X1+d, Y1+d, X2+d, Y2+d, x+c, y+c);//+b1;
				auto w2 = edgeFunction(X2+d, Y2+d, X0+d, Y0+d, x+c, y+c);//+b2;
				if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) { return true; }
				else{ return false; }
			};
		};

		//Sort vertices in asc order:
		if( y0 > y1 ){ std::swap(x0, x1); std::swap(y0, y1); }
		if( y0 > y2 ){ std::swap(x0, x2); std::swap(y0, y2); }
		if( y1 > y2 ){ std::swap(x1, x2); std::swap(y1, y2); }

		/*if( x2 < x1 ){ std::swap(x1, x2); std::swap(y1, y2); }

		auto xmin = std::min({x0, x1, x2});
		auto ymin = std::min({y0, y1, y2});
		auto xmax = std::max({x0, x1, x2});
		auto ymax = std::max({y0, y1, y2});

		auto iI0 = isInside(x0, y0, x1, y1, x2, y2);
		for(int x=xmin; x<=xmax; ++x)
		{
		for(int y=ymin; y<=ymax; ++y)
		{
		if(iI0(x, y)){ backbuffer(x, y) = f(backbuffer(x, y)); }
		}
		}

		return;*/

		auto fillBottomFlatTriangle = [&](T X0, T Y0, T X1, T Y1, T X2, T Y2, auto iI)
		{
			if(X1 > X2){ std::swap(X1, X2); std::swap(Y1, Y2); }

			auto ylim = (int)std::min(Y1, Y2);

			auto dx1 =  abs(X1-X0); auto sx1 = X0 < X1 ? 1 : -1;
			auto dy1 = -abs(Y1-Y0); auto sy1 = Y0 < Y1 ? 1 : -1;
			auto dx2 =  abs(X2-X0); auto sx2 = X0 < X2 ? 1 : -1;
			auto dy2 = -abs(Y2-Y0); auto sy2 = Y0 < Y2 ? 1 : -1;

			if(dx1 == 0 && dy1 == 0 && dx2 == 0 && dy2 == 0){ return; }

			auto err1 = dx1 + dy1; auto e21 = 0.0f; 
			auto err2 = dx2 + dy2; auto e22 = 0.0f;
			auto x01 = X0;
			auto x02 = X0;
			auto y01 = Y0;
			auto y02 = Y0;
			for (;;){
				for(;;)
				{
					e21 = 2 * err1 + 0.5f;
					if(e21 >= dy1){ err1 += dy1; x01 += sx1; }
					if(e21 <= dx1){ err1 += dx1; break; }
				}
				for(;;)
				{
					e22 = 2 * err2 + 0.5f;
					if(e22 >= dy2){ err2 += dy2; x02 += sx2; }
					if(e22 <= dx2){ err2 += dx2; break; }
				}
				hline(x01, x02, y01, iI, f);
				y01 += sy1;
				y02 += sy2;
				if ((int)y01 > (int)ylim) break;
			}
		};

		auto fillTopFlatTriangle = [&](T X0, T Y0, T X1, T Y1, T X2, T Y2, auto iI)
		{
			if(X0 > X1){ std::swap(X1, X0); std::swap(Y1, Y0); }
			auto ylim = (int)std::max(Y0, Y1);
			auto dx1 =  abs(X0 - X2); auto sx1 = X2 < X0 ? 1 : -1;
			auto dy1 = -abs(Y0 - Y2); auto sy1 = Y2 < Y0 ? 1 : -1;
			auto dx2 =  abs(X1 - X2); auto sx2 = X2 < X1 ? 1 : -1;
			auto dy2 = -abs(Y1 - Y2); auto sy2 = Y2 < Y1 ? 1 : -1;

			if(dx1 == 0 && dy1 == 0 && dx2 == 0 && dy2 == 0){ return; }

			auto err1 = dx1 + dy1; auto e21 = 0.0f; 
			auto err2 = dx2 + dy2; auto e22 = 0.0f;

			auto x01 = X2;
			auto x02 = X2;
			auto y01 = Y2;
			auto y02 = Y2;
			for (;;){
				for(;;)
				{
					e21 = 2 * err1 + 0.5f;
					if(e21 >= dy1){ err1 += dy1; x01 += sx1; }
					if(e21 <= dx1){	err1 += dx1; break; }
				}
				for(;;)
				{
					e22 = 2 * err2 + 0.5f;
					if(e22 >= dy2){ err2 += dy2; x02 += sx2; }
					if(e22 <= dx2){ err2 += dx2; break; }
				}
				hline(x01, x02, y01, iI, f);
				y01 += sy1;
				y02 += sy2;
				if ((int)y01 <= (int)ylim) break;
			}
		};

		auto xx1 = x1;
		auto yy1 = y1;
		auto xx2 = x2;
		auto yy2 = y2;
		if( x2 < x1 ){ std::swap(xx1, xx2); std::swap(yy1, yy2); }
		auto iI = isInside(x0, y0, xx1, yy1, xx2, yy2);

		auto sx = (x0+x1+x2)/(T)3;
		auto sy = (y0+y1+y2)/(T)3;

		auto r = std::max(x2-x0, y2-y0);
		const T scale = (T)1.0 + (T)(10.0 / r);
		auto ex0 = (x0 - sx) * scale + sx;
		auto ex1 = (x1 - sx) * scale + sx;
		auto ex2 = (x2 - sx) * scale + sx;

		auto ey0 = (y0 - sy) * scale + sy;
		auto ey1 = (y1 - sy) * scale + sy;
		auto ey2 = (y2 - sy) * scale + sy;

		/*auto wh = [](auto ){ return bgr8(255, 255, 255); };
		line(ex0, ey0, ex1, ey1, wh);
		line(ex1, ey1, ex2, ey2, wh);
		line(ex2, ey2, ex0, ey0, wh);*/

		if((int)y1 == (int)y2)
		{
			fillBottomFlatTriangle(ex0, ey0, ex1, ey1, ex2, ey2, iI);
		}
		else if((int)y0 == (int)y1)
		{
			fillTopFlatTriangle(ex0, ey0, ex1, ey1, ex2, ey2, iI);
		} 
		else
		{
			fillBottomFlatTriangle(ex0, ey0, ex1, ey1, ex2, ey2, iI);
			fillTopFlatTriangle   (ex0, ey0, ex1, ey1, ex2, ey2, iI);
		}
	}

	template<typename F>
	void ellipse(int xm, int ym, int a, int b, F&& f)
	{
		const int w = backbuffer.w();
		const int h = backbuffer.h();
		if(xm-a < 0 || ym-b < 0 || xm+a > w-1 || ym+b > h-1){ return; }
		long x = -a, y = 0; /* II. quadrant from bottom left to top right */
		long e2 = b, dx = (1+2*x)*e2*e2; /* error increment */
		long dy = x*x, err = dx+dy; /* error of 1.step */
		do
		{
			backbuffer(xm+x, ym-y) = f(backbuffer(xm+x, ym-y)); /* III. Quadrant */
			backbuffer(xm-x, ym-y) = f(backbuffer(xm-x, ym-y)); /* IV. Quadrant */
			backbuffer(xm-x, ym+y) = f(backbuffer(xm-x, ym+y)); /* I. Quadrant */
			backbuffer(xm+x, ym+y) = f(backbuffer(xm+x, ym+y)); /* II. Quadrant */
			e2 = 2*err;
			if (e2 >= dx) { x++; err += dx += 2*(long)b*b; } /* x step */
			if (e2 <= dy) { y++; err += dy += 2*(long)a*a; } /* y step */
		} while (x <= 0);
		while (y++ < b) { /* to early stop for flat ellipses with a=1, */
			backbuffer(xm, ym+y) = f(backbuffer(xm, ym+y)); /* -> finish tip of ellipse */
			backbuffer(xm, ym-y) = f(backbuffer(xm, ym-y)); 
		}
	}

	template<typename F>
	void filledellipse(int xm, int ym, int a, int b, F&& f)
	{
		const int w = backbuffer.w();
		const int h = backbuffer.h();
		if(xm-a < 0 || ym-b < 0 || xm+a > w-1 || ym+b > h-1){ return; }
		long x = -a, y = 0; /* II. quadrant from bottom left to top right */
		long e2 = b, dx = (1+2*x)*e2*e2; /* error increment */
		long dy = x*x, err = dx+dy; /* error of 1.step */
		do
		{
			auto mn = std::min(xm-x, xm+x);
			auto mx = std::max(xm-x, xm+x);
			for(auto x_ = mn; x_ <= mx; ++x_)
			{
				backbuffer(x_, ym-y) = f(backbuffer(x_, ym-y)); //Quadrant III-IV
				backbuffer(x_, ym+y) = f(backbuffer(x_, ym+y)); //Quadrant I-II
			}
			e2 = 2*err;
			if (e2 >= dx) { x++; err += dx += 2*(long)b*b; } /* x step */
			if (e2 <= dy) { y++; err += dy += 2*(long)a*a; } /* y step */
		} while (x <= 0);
		while (y++ < b) { /* to early stop for flat ellipses with a=1, */
			backbuffer(xm, ym+y) = f(backbuffer(xm, ym+y)); /* -> finish tip of ellipse */
			backbuffer(xm, ym-y) = f(backbuffer(xm, ym-y)); 
		}
	}

	void filledellipse(int xm, int ym, int a, int b, Color8 col){ filledellipse(xm, ym, a, b, [=](auto){ return col; }); }
	void ellipse      (int xm, int ym, int a, int b, Color8 col){ ellipse      (xm, ym, a, b, [=](auto){ return col; }); }
};

struct MainWindow
{
	PlatformWindowData	window;
	SoftwareRenderer	renderer;

#ifdef _WIN32
	HDC					hdc;
	HBITMAP				bmp;
	HGDIOBJ				oldbmp;
#else
	GC					gc;
	Pixmap				bmp;
#endif

	std::function<void(void)> onAppStep, onAppExit;
	std::function<void(int, int, bool)> onAppResize;
	std::function<void(SoftwareRenderer&)> onAppRender; 
	MainWindow():onAppStep{[]{}}, onAppExit{[]{}}, onAppResize{[](int, int, bool){}}, onAppRender{[](SoftwareRenderer&){}}
	{
#ifdef _WIN32
		hdc = 0; bmp = 0; oldbmp = 0;
#else
		bmp = 0;
#endif
	}

	auto width() const { return window.size.w; }
	auto height() const { return window.size.h; }

	template<typename FInit>
	bool open(utf8string const& title, pos2<int> pos_, size2<int> size_, bool Decorated_, bool FullScreen_, FInit&& finit)
	{
		using namespace MainWindowDetails;
		if( !window.open(title, pos_, size_, Decorated_, FullScreen_) ){ return false; }

		relay.onRender = [&]{ this->onRender(); };
		relay.onExit   = [&]{ this->onExit(); };
		relay.onResize = [&](int w, int h, bool m){ this->onResize(w, h, m); };

		renderer.init(width(), height());
		allocate_buffers();
#ifdef _WIN32
#else
		gc = XCreateGC(window.display, window.handle, 0, 0);
#endif
		onResize(width(), height(), false);

		if( !finit() ){ return false; }
		window.show();
		window.loop(onAppStep);
		renderer.close();
		return window.close();
	}

	void quit(){ window.quit(); }

	template<typename F> void      exitHandler(F&& f){ onAppExit   = std::forward<F>(f); }
	template<typename F> void      idleHandler(F&& f){ onAppStep   = std::forward<F>(f); }
	template<typename F> void    renderHandler(F&& f){ onAppRender = std::forward<F>(f); }
	template<typename F> void    resizeHandler(F&& f){ onAppResize = std::forward<F>(f); }
	template<typename F> void     mouseHandler(F&& f){ MainWindowDetails::relay.onMouseEvent    = std::forward<F>(f); }
	template<typename F> void  keyboardHandler(F&& f){ MainWindowDetails::relay.onKeyboardEvent = std::forward<F>(f); }
	void onRender()
	{
		//printf("OnRender\n");
		if(window.size.area() == 0){ return; }
		onAppRender(renderer);

#ifdef _WIN32
		PAINTSTRUCT ps;
		auto paintdc = BeginPaint(window.handle, &ps);
		
		HDC     tmpdc     = CreateCompatibleDC(hdc);
		HBITMAP tmpbmp    = CreateBitmap(width(), height(), 1, 32, renderer.backbuffer.data.data());
		HGDIOBJ oldtmpbmp = SelectObject(tmpdc, tmpbmp);
		
		BitBlt(paintdc, 0, 0, width(), height(), tmpdc, 0, 0, SRCCOPY);

		SelectObject(tmpdc, oldtmpbmp);
		DeleteObject(tmpbmp);
		DeleteDC(tmpdc);

		EndPaint(window.handle, &ps);
		//ValidateRect(window.handle, NULL);
#else
		//renderer.forall_pixels([](auto x, auto y, auto){ return color8(0, 0, 64); });
		XImage* image = XCreateImage(window.display, window.visual, window.depth, ZPixmap, 0, (char*)renderer.backbuffer.data.data(), width(), height(), 32, 0);
		XPutImage(window.display, bmp, gc, image, 0, 0, 0, 0, width(), height());
		XFree(image);
		XCopyArea(window.display, bmp, window.handle, gc, 0, 0, width(), height(), 0, 0);
		XFlush(window.display);
#endif
	}

	void allocate_buffers()
	{
#ifdef _WIN32
		auto dcw = GetDC(window.handle);
		hdc    = CreateCompatibleDC(dcw);
		bmp    = CreateCompatibleBitmap(dcw, width(), height());
		oldbmp = SelectObject(hdc, bmp);
		ReleaseDC(window.handle, dcw);
#else
		bmp = XCreatePixmap(window.display, window.handle, width(), height(), window.depth);
#endif
	}

	void free_buffers()
	{
#ifdef _WIN32
		if(hdc)
		{
			SelectObject(hdc, oldbmp);
			DeleteObject(bmp);
			DeleteDC(hdc);
		}
#else
		if(bmp){ XFreePixmap(window.display, bmp); }
#endif
	}

	void onResize(int w, int h, bool)
	{
		//printf("resize\n");
		bool m = (w == width()) && (h == height());
		if(!m)
		{
			free_buffers();
			renderer.resize(w, h);
			allocate_buffers();
			window.size.w = w; window.size.h = h; 
		}
		onAppResize(w, h, m);
		if(!m)
		{
			//printf("stepping and redrawing\n");
			//onAppStep();
			//window.redraw();
		}
	}

	void onExit()
	{
		onAppExit();
	}
};
