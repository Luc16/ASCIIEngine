#include "ASCIIEngine.h"


static struct termios ORIGINAL_TERMIOS;

namespace aen {

    // ASCIIEngine::ASCIIEngine(int width, int height, std::string profileID, int fontSize, std::string appName): 
    // m_width(width), m_height(height), m_profileID(profileID), m_appName(appName) {
    //     m_screenBuffer.resize(height);
    //     for (auto& line: m_screenBuffer){
    //         line.resize(width);
    //         for (auto& sc: line)
    //             sc.c = ' ';
    //     }
    //     SetFontSize(fontSize);
    //     system("clear");
    //     //set console size
    //     std::cout << "\e[8;" << m_height + 1 << ";" << m_width << "t";
    // }
    
    ASCIIEngine::~ASCIIEngine(){
        SetFontSize(12);
        // Show cursor
        std::cout << "\e[?25h";
        //Set console size
        std::cout << "\e[8;" << 24 << ";" << 80 << "t";
        system("clear");
    }

    void ASCIIEngine::MoveCursor(int row, int col){
        std::cout << "\033[" << row << ";" << col << "H";
    }

    void ASCIIEngine::SetFontSize(int fontSize){
    
        std::string s = "gsettings set org.gnome.Terminal.Legacy.Profile:/org/gnome/terminal/legacy/profiles:/:" + m_profileID + 
        "/ font 'Monospace Regular " + std::to_string(fontSize) + "'";
        system(s.c_str());
    }

    void ASCIIEngine::ConstructConsole(int width, int height, std::string profileID, int fontSize, std::string appName){
        m_width = width;
        m_height = height;
        m_profileID = profileID;
        m_appName = appName;

        m_screenBuffer.resize(height);
        for (auto& line: m_screenBuffer){
            line.resize(width);
            for (auto& sc: line)
                sc.c = ' ';
        }
        SetFontSize(fontSize);
        system("clear");
        // Hide cursor
        std::cout << "\e[?25l";
        // Set console size
        std::cout << "\e[8;" << m_height + 1 << ";" << m_width << "t";
    }

    void ASCIIEngine::Run(){
        m_bAtomRuning = true;
        std::thread t(&ASCIIEngine::GameThread, this);

        t.join();
    }

    void ASCIIEngine::GameThread(){
        if (!OnCreate()) m_bAtomRuning = false;


        auto t1 = std::chrono::system_clock::now();
        auto t2 = std::chrono::system_clock::now();

        while (m_bAtomRuning){
            // Handle delta time
            t2 = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsedTime = t2 - t1;
            t1 = t2;
            float fDelta = elapsedTime.count();

            char cKey = GetKey();
            if (cKey != '\0') system("clear");

            if (!GameLoop(fDelta, cKey)) m_bAtomRuning = false;

            std::cout << "\033]0;ASCII Engine - " << m_appName << " - FPS: "<< 1.0f/fDelta <<"\007" << std::flush;
            DrawScreen();
        }
        
        OnDestroy();
    

    }


    void ASCIIEngine::Draw(int x, int y, char c, COLOR fgColor, COLOR bgColor, STYLE style){
        if (x >= 0 && x < m_width && y >= 0 && y < m_height){
            m_screenBuffer[y][x].c = c;
            m_screenBuffer[y][x].fgColor = fgColor;
            m_screenBuffer[y][x].bgColor = bgColor;
            m_screenBuffer[y][x].style = style;
            
        }
    }

    void ASCIIEngine::DrawString(int x, int y, std::string s, COLOR fgColor, COLOR bgColor){
        for (std::string::size_type i = 0; i < s.length(); i++){
            int nx = x + i;
            Clip(nx, y);
            Draw(nx, y, s[i], fgColor, bgColor);
        }
    }

    void ASCIIEngine::Clip(int& x, int& y){
        if (x < 0) x = 0;
		if (x >= m_width) x = m_width;
		if (y < 0) y = 0;
		if (y >= m_height) y = m_height;
    }

    void ASCIIEngine::Fill(int x0, int y0,int x1,int y1, COLOR bgColor){
        Clip(x0, y0); 
        Clip(x1, y1);
         for (int i = y0; i < y1; i++)
            for (int j = x0; j < x1; j++){
                m_screenBuffer[i][j].c = ' ';
                m_screenBuffer[i][j].bgColor = bgColor;
            }
    }

    void ASCIIEngine::FillScreen(COLOR bgColor){
         for (int i = 0; i < m_height; i++)
            for (int j = 0; j < m_width; j++){
                m_screenBuffer[i][j].c = ' ';
                m_screenBuffer[i][j].bgColor = bgColor;
            }
    }

    void ASCIIEngine::DrawScreen(){
        MoveCursor(1, 1);
        for (auto& line: m_screenBuffer){
            for (auto& sc: line){
                if (sc.bgColor) std::cout << "\033[" << sc.bgColor << "m";
                if (sc.fgColor) std::cout << "\033[" << sc.fgColor << "m";
                if (sc.style) std::cout << "\033[" << sc.style << "m";
                std::cout << sc.c << "\033[0m";
            }
            std::cout << std::endl;
        }
        MoveCursor(1, 1);
    }

    static void TerminalReset(){
        tcsetattr(0, TCSANOW, &ORIGINAL_TERMIOS);

    }

    void ASCIIEngine::TerminalSetRaw(){
        struct termios newTermios;

        tcgetattr(0, &ORIGINAL_TERMIOS);
        memcpy(&newTermios, &ORIGINAL_TERMIOS, sizeof(newTermios));

        atexit(TerminalReset);
        cfmakeraw(&newTermios);
        tcsetattr(0, TCSANOW, &newTermios);
    }

    char ASCIIEngine::GetKey(){
        TerminalSetRaw();
        struct timeval tv = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        char cs[5];
        cs[0] = '\0';
        int r = 1;

        if (select(1, &fds, NULL, NULL, &tv) > 0)
            r = read(0, &cs, sizeof(cs));

        TerminalReset();
        return cs[r - 1];
    }

}