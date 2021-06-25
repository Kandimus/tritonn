
#include <string>


class rBashColor
{
public:
	enum Color
	{
		UNUSED = -1,
		BLACK = 0,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE,
	};

	enum Style
	{
		NONE      = 0x00,
		NORMAL    = 0x01,
		BOLD      = 0x02,
		UNDERLINE = 0x04,
		BLINK     = 0x08,
		INVERT    = 0x10,
		INVISIBLE = 0x20,
	};

	#define COLOR_RESET "\x1b[0m"

	#define APPLY_STYLE(x, y) if (style & (x)) { ascii += (ascii.size() ? std::string(";") : std::string("")) + #y; }

	static std::string getColor(Color txt, Color bg, Style style, bool iscolored)
	{
		static const std::string escape     = "\x1b[";
		static const std::string colors[8]  = {"30", "31", "32", "33", "34", "35", "36", "37"};
		static const std::string bground[8] = {"40", "41", "42", "43", "44", "45", "46", "47"};

		std::string ascii = "";

		if (!iscolored) {
			return ascii;
		}

		APPLY_STYLE(Style::NORMAL   , 10);
		APPLY_STYLE(Style::BOLD     ,  1);
		APPLY_STYLE(Style::UNDERLINE,  4);
		APPLY_STYLE(Style::BLINK    ,  5);
		APPLY_STYLE(Style::INVERT   ,  7);
		APPLY_STYLE(Style::INVISIBLE,  8);

		if (txt != Color::UNUSED) {
			ascii += (ascii.size() ? std::string(";") : std::string("")) + colors[txt];
		}

		if (bg != Color::UNUSED) {
			ascii += (ascii.size() ? std::string(";") : std::string("")) + bground[txt];
		}

		return ascii.size() ? escape + ascii + "m" : ascii;
	}

	static std::string reset(bool iscolored)
	{
		return iscolored ? COLOR_RESET : "";
	}

	static std::string green(bool iscolored)
	{
		return getColor(rBashColor::GREEN, rBashColor::UNUSED, rBashColor::NONE, iscolored);
	}

	static std::string red(bool iscolored)
	{
		return getColor(rBashColor::RED, rBashColor::UNUSED, rBashColor::NONE, iscolored);
	}

	std::string setColor(Color txt, Color bg, Style style, const std::string str, bool iscolored, bool isescaped)
	{
		std::string result = str;
		std::string escape = getColor(txt, bg, style, iscolored);

		if (escape.size()) {
			result = escape + result;

			if (isescaped) {
				result += COLOR_RESET;
			}
		}

		return result;
	}
};
