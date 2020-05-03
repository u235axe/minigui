#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>

/*#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"*/
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "graphics_base.h"

struct StbFont
{
	std::string filename;
	stbtt_fontinfo font;
	std::vector<unsigned char> fontfilebuffer;
	int ascent, descent, linegap;
	float max_asc, max_desc;//both positive, measured from baseline

	int height(float height_to_scale_for) const
	{
		float scale = stbtt_ScaleForPixelHeight(&font, height_to_scale_for);
		return (int)(scale * (ascent - descent));
	}

	int get_dx(float height) const
	{
		float scale = stbtt_ScaleForPixelHeight(&font, height);
		int advance, lsb;
		stbtt_GetCodepointHMetrics(&font, 'A', &advance, &lsb);
		return (int)(advance * scale);
	}

	int xdist(int nch, float height) const
	{
		int dx = get_dx(height);
		return (int)(nch * dx);
	}

	bool init(std::string const& fn)
	{
		filename = fn;
		std::ifstream fontfile(filename, std::ios::binary );
		if(!fontfile.is_open()){ std::cout << "Cannot open font file: " << filename << "\n"; return false; }
		fontfilebuffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(fontfile), {});

		int res = stbtt_InitFont(&font, (const unsigned char*)fontfilebuffer.data(), 0);
		if(res == 0){ std::cout << "stbtt_InitFont failed\n"; return false; }

		stbtt_GetFontVMetrics(&font, &ascent, &descent, &linegap);

		//font metrics seems to be unreliable in some cases so we measure all chars before rendering to get bounds:
		//measure font max asc, desc:
		max_asc  = 0;
		max_desc = 0;
		auto high_chars = utf8s(u8"fhkltÁÉÍÓŐÚŰ[{()}]|").to_codepoints();
		auto  low_chars = utf8s(u8"Qfgjpqty[{()}]|"    ).to_codepoints();

		auto measure_char = [&](auto ch)
		{
			int x0, y0, x1, y1;
			stbtt_GetCodepointBitmapBox(&font, ch, 1.0f, 1.0f, &x0, &y0, &x1, &y1);
			max_asc  = (float)std::min(max_asc,  (float)y0);
			max_desc = (float)std::max(max_desc, (float)y1);
		};
		for(auto ch : high_chars){ measure_char(ch); }
		for(auto ch :  low_chars){ measure_char(ch); }

		max_asc  = -max_asc;

		std::cout << "Loaded font file: " << filename << "\n";
		std::cout << "Ascent / descent: " << max_asc << ", " << max_desc << "\n";
		return true;
	}
};

int get_advance(wchar_t ch, StbFont const& font, float height)
{
	float scale = stbtt_ScaleForPixelHeight(&font.font, height);
	int advance, lsb;
	stbtt_GetCodepointHMetrics(&font.font, ch, &advance, &lsb);
	return (int)(advance * scale);
}

//assumes monospace, assumes no newline
template<typename Str>
size2<int> measure_small_string_monospace(Str const& str, StbFont const& font, float height)
{
	auto cps = str.to_codepoints();
	float scale = stbtt_ScaleForPixelHeight(&font.font, height);
	int baseline = (int)(font.ascent * scale);
	int advance, lsb;
	stbtt_GetCodepointHMetrics(&font.font, 'A', &advance, &lsb);
	int dx = (int)(advance * scale);
	int h = (int)((font.ascent - font.descent)*scale);
	int w = (int)(dx * cps.size());
	return {w, h};
}

//assumes monospace, assumes no newline
template<typename Str>
PrerenderedText render_small_string_monospace(Str const& str, StbFont const& font, float height)
{
	PrerenderedText rt;
	if(height < 3.0f){ return rt; }
	auto  cps   = str.to_codepoints();
	int   n     = (int)cps.size();
	float scale = stbtt_ScaleForPixelHeight(&font.font, height);

	int advance = 0, leftsidebearing = 0;
	{
		char32_t ch0 = (int)'A';
		if(cps.size() > 0){ ch0 = cps[0]; }
		stbtt_GetCodepointHMetrics(&font.font, ch0, &advance, &leftsidebearing);
	}

	auto dw = (int)(advance * scale);                             //the char spacing
	int x00 = (int)(leftsidebearing * scale) + 1*dw;              //1 extra char space at left
	int w   = x00 + n * dw + 1*dw;                                //2 char extra width at edges
	int h   = (int)((font.max_asc + font.max_desc) * scale + 2);  //2 pixel extra space due to rounding
	int baseline = (int)(font.max_asc * scale + 1);				  //baseline position measured from top, added 1 to compensate rounding
	rt.baseline  = baseline;
	rt.text_align_box.y = baseline - (int)(font.max_asc * scale); //top of highest char
	rt.text_align_box.x = x00;                                    //left align edge, character may extend more to the left
	rt.text_align_box.h = h;
	rt.dh               = (int)((font.max_desc + font.max_asc + font.linegap) * scale); // total height to next baseline
	rt.resize(w, h);

	if(cps.size() > 0)
	{
		int chpos = 0;
		auto ch = cps[chpos];
		int last_x = x00;
		while(ch != 0)
		{	
			int xpos = (int)(x00 + dw * chpos);
			
			int x0, y0, x1, y1;
			stbtt_GetCodepointBitmapBox(&font.font, ch, scale, scale, &x0, &y0, &x1, &y1);

			int advance, leftsidebearing;
			stbtt_GetCodepointHMetrics(&font.font, ch, &advance, &leftsidebearing);
			//auto lsb_scaled = (int)(leftsidebearing * scale);

			stbtt_MakeCodepointBitmap(&font.font, &rt.img[(baseline + y0)*w+xpos+x0], x1-x0, y1-y0, w, scale, scale, ch);
			last_x = (ch == 32 ? xpos + dw : xpos + x1);
			++chpos;
			ch = cps[chpos];
		}
		rt.text_align_box.w = last_x - x00;
	}else{ rt.text_align_box.w = dw; }

	return rt;
}

template<typename C>
Image2<C> recolor(Image2<unsigned char> const& img, C const& bkcolor, C const& fgcolor )
{
	Image2<C> res; res.resize(img.size());
	const int N = img.size().area();
	
	for(int i=0; i<N; ++i)
	{
		auto f = [A = img[i]](unsigned char bk, unsigned char fg){ return rescale<unsigned char, unsigned char, float>((unsigned char)0, (unsigned char)255, A, bk, fg); };
		res[i] = C{ f(bkcolor.b, fgcolor.b), f(bkcolor.g, fgcolor.g), f(bkcolor.r, fgcolor.r), f(bkcolor.a, fgcolor.a) };
	}
	return res;
}

/*template<typename T, int ch = sizeof(T) / sizeof(unsigned char), typename Q = std::enable_if_t<(ch > 0 && ch <= 4 && std::is_integral_v<T>), void>>
void save_image_png(std::string const& fn, Image2<unsigned char> const& img)
{
	stbi_write_png(fn.c_str(), img.w(), img.h(), ch, img.data.data(), img.w()*ch);
}

void save_image_png(std::string const& fn, Image2<Color8> const& img)
{
	stbi_write_png(fn.c_str(), img.w(), img.h(), 4, img.data.data(), img.w()*4);
}

Image2<Color8> load_image(std::string const& fn)
{
	Image2<Color8> res; int w = 0, h = 0, comp;
	auto ptr = stbi_load(fn.c_str(), &w, &h, &comp, 4);
	if(!ptr || w<0 || h<0){ return res; }
	res.resize({w, h});
	auto ptr2 = reinterpret_cast<Color8*>(ptr);
	std::copy(ptr2, ptr2+(size_t)w*(size_t)h, res.data.begin());
	stbi_image_free(ptr);
	return res;
}*/