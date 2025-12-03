#include "st7735s.h"

#define IS_LCD_OFFSET 1 //If the LCD is not perfect on two corners, then set this to 0 and try again, or set the LCD offsets.

#if IS_LCD_OFFSET
    #define LCD_X_OFFSET 2
    #define LCD_Y_OFFSET 1
#else
    #define LCD_X_OFFSET 0
    #define LCD_Y_OFFSET 0
#endif

CST7735SDisplay::CST7735SDisplay(CSPIMaster &SPIMaster, CGPIOPin &DCPin, CGPIOPin &RSTPin): m_SPIMaster(SPIMaster), m_DCPin(DCPin), m_RSTPin(RSTPin)
{
    m_XOffset = LCD_X_OFFSET;
    m_YOffset = LCD_Y_OFFSET;
}

void CST7735SDisplay::SendCommand(unsigned char uchCmd)
{
    m_DCPin.Write(0);
    m_SPIMaster.Write(0, &uchCmd, 1);
}

void CST7735SDisplay::SendData(unsigned char uchData)
{
    m_DCPin.Write(1);
    m_SPIMaster.Write(0, &uchData, 1);
}

void CST7735SDisplay::SendDataBuffer(const unsigned char *puchBuf, size_t nLen)
{
    m_DCPin.Write(1);
    m_SPIMaster.Write(0, puchBuf, nLen);
}

void CST7735SDisplay::Init()
{
    // Hardware reset
    m_RSTPin.Write(0);
    CTimer::SimpleMsDelay(50);
    m_RSTPin.Write(1);
    CTimer::SimpleMsDelay(150);

    // Software reset
    SendCommand(0x01);
    CTimer::SimpleMsDelay(150);

    // Sleep out
    SendCommand(0x11);
    CTimer::SimpleMsDelay(150);

    // 16-bit color mode
    SendCommand(0x3A);
    SendData(0x05);

    // Display on
    SendCommand(0x29);
    CTimer::SimpleMsDelay(50);
}

void CST7735SDisplay::SetAddressWindow(unsigned short usX0, unsigned short usY0, unsigned short usX1, unsigned short usY1)
{
    SendCommand(0x2A);
    SendData(usX0 >> 8); SendData(usX0 & 0xFF);
    SendData(usX1 >> 8); SendData(usX1 & 0xFF);

    SendCommand(0x2B);
    SendData(usY0 >> 8); SendData(usY0 & 0xFF);
    SendData(usY1 >> 8); SendData(usY1 & 0xFF);

    SendCommand(0x2C);
}

void CST7735SDisplay::DrawPixel(int nX, int nY, unsigned short usColor)
{
    if(nX < 0 || nX >= 128 || nY < 0 || nY >= 160) return;
    m_FrameBuffer[nY][nX] = usColor;
}

void CST7735SDisplay::Refresh()
{
    SetAddressWindow(m_XOffset, m_YOffset, 127 + m_XOffset, 159 + m_YOffset);

    for(int y = 0; y < 160; y++)
    {
        for(int x = 0; x < 128; x++)
        {
            u16 color = m_FrameBuffer[y][x];
            unsigned char data[2] = { (color >> 8) & 0xFF, color & 0xFF };
            SendDataBuffer(data, 2);
        }
    }
}

void CST7735SDisplay::FillScreen(unsigned short usColor)
{
    for(int y = 0; y < 160 ; y++)
    {
        for(int x = 0; x < 128; x++)
        {
            m_FrameBuffer[y][x] = usColor;
        }
    }
    Refresh();
}

void CST7735SDisplay::DrawChar(int nX, int nY, char c, unsigned short fg, unsigned short bg, TFont fontSel)
{
    unsigned char uc = (unsigned char)c;

    if (uc < fontSel.first_char || uc > fontSel.last_char)
        return;

    const int width  = fontSel.width;
    const int height = fontSel.height;

    const u8* font = (const u8*)fontSel.data;

    const int bytes_per_row  = (width + 7) / 8;
    const int bytes_per_char = bytes_per_row * height;

    unsigned char_index = (uc - fontSel.first_char) * bytes_per_char;
    const u8* glyph = font + char_index;

    for (int row = 0; row < height; row++)
    {
        const u8* rowptr = glyph + row * bytes_per_row;

        for (int col = 0; col < width; col++)
        {
            int byte_index = col / 8;
            int bit_index  = col % 8;

            bool pixel_on = (rowptr[byte_index] >> bit_index) & 1;
            int draw_col = width - 1 - col;

            DrawPixel(nX + draw_col, nY + row, pixel_on ? fg : bg);
        }
    }
}

void CST7735SDisplay::DrawString(int nX, int nY, const char* str, unsigned short usColor, unsigned short usBgColor, TFont font)
{
    int x = nX;
    int y = nY;

    while (*str)
    {
        if (*str == '1'){
            DrawChar(x, y, *str, 0xF800, usBgColor, font);
        }else{
            DrawChar(x, y, *str, usColor, usBgColor, font);
        }
        
        x += font.width;

        if (x + font.width > 128)
        {
            x = 0;
            y += font.height;
        }
        str++;
    }
}

void CST7735SDisplay::DrawCharScaled(int nX, int nY, char c, unsigned short fg, unsigned short bg, int scale, TFont fontSel)
{
    unsigned char uc = (unsigned char)c;
    if (uc < fontSel.first_char || uc > fontSel.last_char)
        return;

    const int width  = fontSel.width;
    const int height = fontSel.height;
    const u8* font   = (const u8*)fontSel.data;

    const int bytes_per_row  = (width + 7) / 8;
    const int bytes_per_char = bytes_per_row * height;
    unsigned char_index = (uc - fontSel.first_char) * bytes_per_char;
    const u8* glyph = font + char_index;

    for (int row = 0; row < height; row++)
    {
        const u8* rowptr = glyph + row * bytes_per_row;

        for (int col = 0; col < width; col++)
        {
            int byte_index = col / 8;
            int bit_index  = 7 - (col % 8); // adjust for MSB-left or LSB-left
            bool pixel_on = (rowptr[byte_index] >> bit_index) & 1;

            unsigned short color = pixel_on ? fg : bg;

            // Draw a SCALE x SCALE block
            for (int dy = 0; dy < scale; dy++){
                for (int dx = 0; dx < scale; dx++){
                    DrawPixel(nX + col*scale + dx, nY + row*scale + dy, color);
                }
            }
        }
    }
}

void CST7735SDisplay::DrawStringScaled(int nX, int nY, const char* str, unsigned short fg, unsigned short bg, int scale, TFont fontSel)
{
    int x = nX;
    int y = nY;

    while (*str)
    {
        if (*str == '1'){
            DrawCharScaled(x, y, *str, 0xF800, bg, scale, fontSel);
        }else{
            DrawCharScaled(x, y, *str, fg, bg, scale, fontSel);
        }
        x += fontSel.width * scale;

        if (x + fontSel.width*scale > 128)
        {
            x = 0;
            y += fontSel.height*scale;
        }
        str++;
    }
}
