#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x7bef

enum ColorDefs	{
		Black			= 0x0000,
		White			= 0xFFFF,
		BrightRed		= 0xF800,
		BrightGreen		= 0x07E0,
		BrightBlue		= 0x001F,
		BrightCyan		= 0x07FF,
		BrightMagenta		= 0xF81F,
		Yellow			= 0xFFF0,
		BrightYellow		= 0xFFE0,
		LightYellow		= 0xFFF2,
		Gold			= 0xFEA0,
		Blue			= 0x0010,
		Green			= 0x0400,
		Cyan			= 0x0410,
		Red			= 0x8000,
		Magenta			= 0x8010,
		Brown			= 0xFC00,
		LightGray		= 0x8410,
		DarkGray		= 0x4208,
		LightBlue		= 0x841F,
		LightGreen		= 0x87F0,
		LightCyan		= 0x87FF,
		LightRed		= 0xFC10,
		LightMagenta		= 0xFC1F,
		SaddleBrown		= 0x8A22,
		Sienna			= 0xA285,
		Peru			= 0xCC27,
		Burlywood		= 0xDDD0,
		Wheat			= 0xF7BB,
		Tan			= 0xD5B1,
		Orange			= 0xFDC9,
		DarkOrange		= 0xFC60,
		LightOrange		= 0xFE40,
		Gray242			= 0xF79E,
		Gray229			= 0xE73C,
		Gray204			= 0xCE79,
		Gray192			= 0xC618,
		Gray160			= 0xA514,
		Gray128			= 0x8410,
		Gray96			= 0x630C,
		Gray32			= 0x2104,
		Gray10			= 0x0841,
		Copper			= 0xfc4b
};

uint16_t SwapBinary(uint16_t v);


void DisplayDigitBackground(uint8_t * background,uint16_t fg,uint16_t bg );

void DisplayDigit(int digit, uint8_t type, uint16_t digitfg, uint16_t backgroundfg, uint16_t bg );

void drawtext90centred(int16_t ax, char * text, uint16_t fg, uint16_t bg);

void ErrorBackground(uint16_t fg, uint16_t bg);

