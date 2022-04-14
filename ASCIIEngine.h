#include <vector>
#include <thread>
#include <atomic>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

enum COLOR {
    NONE = 0,
    FG_BLACK = 30,
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_MAGENTA = 35,
    FG_CYAN = 36,
    FG_WHITE = 37,

    BG_BLACK = 40,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_YELLOW = 43,
    BG_BLUE = 44,
    BG_MAGENTA = 45,
    BG_CYAN = 46,
    BG_WHITE = 47
};

enum STYLE {
    SNONE = 0,
    BOLD = 1,
    FAINT = 3,
    ITALIC = 4,
    UNDERLINE = 5
};

typedef struct
{
    COLOR fgColor = NONE, bgColor = NONE;
    STYLE style = SNONE;
    char c;

} ScreenChar;

namespace aen {
    class ASCIIEngine
    {
    private:
        std::vector<std::vector<ScreenChar>> m_screenBuffer;
        std::string m_profileID, m_appName;
        std::atomic<bool> m_bAtomRuning;        
        int m_width,m_height;         
    
    public:
        ASCIIEngine() {};
        ~ASCIIEngine();
        void ConstructConsole(int width, int height, std::string profileID, int fontSize = 12, std::string appName = "");

        virtual void Draw(int x, int y, char c = '.', COLOR fgColor = NONE, COLOR bgColor = NONE, STYLE style = SNONE);
        void Fill(int x0, int y0,int x1,int y1, COLOR bgColor = NONE);
        void FillScreen(COLOR bgColor = NONE);
        void DrawString(int x, int y, std::string s, COLOR fgColor = NONE, COLOR bgColor = NONE);

        void Run();
        
        virtual bool GameLoop(float fDelta, char cKey) = 0;
        virtual bool OnCreate() = 0;
        virtual bool OnDestroy() {return true; };

        int getGameWidth(){ return m_width; }
        int getGameHeight(){ return m_height; }
        void setAppName(std::string name){ m_appName = name; }


    private:
        void Clip(int& x, int& y);
        void MoveCursor(int row, int col);
        void SetFontSize(int fontSize);
        void DrawScreen();

        void GameThread();

        void TerminalSetRaw();
        char GetKey();
    };
}

