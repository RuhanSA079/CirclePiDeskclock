// RuhanvdB -> ST7735S LCD driver code.
// Implements a basic LCD interface to draw pixels and some text from a font.
// Implemented a basic, small "framebuffer" to avoid LCD flickering when writing text to the LCD.
// TODO: Build in GPL/MIT license?

#include <circle/spimaster.h>
#include <circle/gpiopin.h>
#include <circle/types.h>
#include <circle/font.h>
#include <circle/timer.h>

class CST7735SDisplay
{
    public:
        CST7735SDisplay(CSPIMaster &SPIMaster, CGPIOPin &DCPin, CGPIOPin &RSTPin);

        void Init();
        void FillScreen(unsigned short usColor);
        void DrawPixel(int nX, int nY, unsigned short usColor);
        void DrawString(int nX, int nY, const char* str, unsigned short usColor, unsigned short usBgColor, TFont font = Font8x16);
        void DrawStringScaled(int nX, int nY, const char* str, unsigned short fg, unsigned short bg, int scale, TFont fontSel = Font8x16);
        void Refresh();
        
    private:
        CSPIMaster &m_SPIMaster;
        CGPIOPin   &m_DCPin;
        CGPIOPin   &m_RSTPin;
        
        u16 m_FrameBuffer[160][128];
        int m_XOffset = 0;
        int m_YOffset = 0;

        
        void SendCommand(unsigned char uchCmd);
        void SendData(unsigned char uchData);
        void SendDataBuffer(const unsigned char *puchBuf, size_t nLen);
        void SetAddressWindow(unsigned short usX0, unsigned short usY0, unsigned short usX1, unsigned short usY1);
        void DrawChar(int nX, int nY, char c, unsigned short usColor, unsigned short usBgColor, TFont fontSel);
        void DrawCharScaled(int nX, int nY, char c, unsigned short fg, unsigned short bg, int scale, TFont fontSel);
};
