#include "ASCIIEngine.h"
#include <fstream>
#include <string>
#include <random>

#define PADDLE_SPEED 60
#define BALL_SPEED 100

class PongPaddle {

    float m_x, m_y;
    int m_direction = 1;
    std::string m_base;
    int m_height, m_width;
    
    public:
    PongPaddle() {}

    PongPaddle(int x, int y, int width, int height): m_x(x), m_y(y), m_height(height), m_width(width) {
        char base[width + 1];
        for (int i = 0; i < width; i ++){
            base[i] = ' ';
        }
        base[width] = '\0';
        m_base = base;
    }

    void ChangeDirection(){
        m_direction *= -1;
    }
    
    void Update(float fDelta, int gameHeight){
        m_y += PADDLE_SPEED*fDelta*m_direction;
        if (m_y < 0) {
            m_y = 0;
            ChangeDirection();
        } else if (m_y + m_height > gameHeight){
            m_y = (float) gameHeight - m_height;
            ChangeDirection();
        } 
    }

    int getX(){ return (int) m_x; }
    int getY(){ return (int) m_y; }
    int getHeight(){ return m_height; }
    int getWidth(){ return m_width; }
    std::string getBase(){ return m_base; }


};


class Ball {

    float m_x, m_y;
    int m_radius;
    public:
    float m_dir[2];

    public:

    Ball(int gameWidth, int gameHeight, float radius): m_radius(radius) {
        ResetBall(gameWidth, gameHeight);
    }

    void BounceX() { m_dir[0] *= -1; }
    void BounceY() { m_dir[1] *= -1; }

    void CollideUpperWalls(int gameHeight){
        if (m_y - m_radius <= 0) {m_y = m_radius; BounceY();} 
        else if (m_y + m_radius >= gameHeight) {m_y = gameHeight - m_radius; BounceY();}
    }

    void ResetBall(int gameWidth, int gameHeight){
        m_x = gameWidth/2;
        m_y = gameHeight/2;

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_real_distribution<> distr(-0.7, 0.7);
        m_dir[1] = (float) distr(gen);
        m_dir[0] = distr(gen) < 0 ? -sqrt(1-m_dir[1]*m_dir[1]): sqrt(1-m_dir[1]*m_dir[1]);
    }

    bool CollideLeftWall(int gameWidth, int gameHeight){
        if (m_x - m_radius <= 0) {m_x = m_radius; ResetBall(gameWidth, gameHeight); return true;}
        return false; 
    }

    bool CollideRightWall(int gameWidth, int gameHeight){
        if (m_x + m_radius >= gameWidth) {m_x = gameWidth - m_radius; ResetBall(gameWidth, gameHeight); return true; }
        return false;
    }

    void CollidePaddleRight(PongPaddle paddle, bool xCollision){
        if (m_y - m_radius > paddle.getY() && m_y + m_radius < paddle.getY() + paddle.getHeight() && m_x + m_radius > paddle.getX()){
            if (xCollision) {
                m_x = paddle.getX() - m_radius;
                BounceX();
                return;
            } 
            float dy1 = m_y - m_radius - paddle.getY();
            float dy2 = paddle.getY() + paddle.getHeight() - m_y - m_radius; 
            BounceY();
            if (dy1 > dy2) {m_y = paddle.getY() + paddle.getHeight() + m_radius; return; }
            m_y = paddle.getY() - m_radius;
        }
        
    }

    void CollidePaddleLeft(PongPaddle paddle, bool xCollision){
        if (m_y - m_radius > paddle.getY() && m_y + m_radius < paddle.getY() + paddle.getHeight() && m_x - m_radius < paddle.getX() + paddle.getWidth()){
            if (xCollision) {
                m_x = paddle.getX() + paddle.getWidth() + m_radius;
                BounceX();
                return;
            } 
            float dy1 = m_y - m_radius - paddle.getY();
            float dy2 = paddle.getY() + paddle.getHeight() - m_y - m_radius; 
            BounceY();
            if (dy1 > dy2) {m_y = paddle.getY() + paddle.getHeight() + m_radius; return; }
            m_y = paddle.getY() - m_radius;
        }
    }

    void Update(float fDelta, int gameWidth, int gameHeight, PongPaddle paddles[2]){
        m_x += BALL_SPEED*fDelta*m_dir[0];
        CollidePaddleLeft(paddles[0], true);
        CollidePaddleRight(paddles[1], true);

        m_y += BALL_SPEED*fDelta*m_dir[1];
        CollidePaddleLeft(paddles[0], false);
        CollidePaddleRight(paddles[1], false);

        CollideUpperWalls(gameHeight);
    }


    int getX(){ return (int) m_x; }
    int getY(){ return (int) m_y; }
    int getRadius(){ return m_radius; }
};


class Pong: public aen::ASCIIEngine {

    PongPaddle paddles[2] = {PongPaddle(), PongPaddle()};
    int scores[2] = {0, 0}, gameWidth, gameHeight;
    std::unique_ptr<Ball> ball;

protected:
    void DrawPaddle(PongPaddle paddle){
        for (int i = 0; i < paddle.getHeight(); i++)
            DrawString(paddle.getX(), paddle.getY() + i, paddle.getBase(), NONE, BG_RED);
    }

    void DrawBall(Ball ball){
        int yc = ball.getY(), xc = ball.getX(), r = ball.getRadius();
        int x = 0;
		int y = r;
		int e = 3 - 2 * r;

		while (y >= x) // only formulate 1/8 of circle
		{
			Draw(xc - x, yc - y/2, ' ', NONE, BG_GREEN);
			Draw(xc - y, yc - x/2, ' ', NONE, BG_GREEN);
			Draw(xc + y, yc - x/2, ' ', NONE, BG_GREEN);
			Draw(xc + x, yc - y/2, ' ', NONE, BG_GREEN);
			Draw(xc - x, yc + y/2, ' ', NONE, BG_GREEN);
			Draw(xc - y, yc + x/2, ' ', NONE, BG_GREEN);
			Draw(xc + y, yc + x/2, ' ', NONE, BG_GREEN);
			Draw(xc + x, yc + y/2, ' ', NONE, BG_GREEN);
			if (e < 0) e += 4 * x++ + 6;
			else e += 4 * (x++ - y--) + 10;
		}


    }

    void DrawLine(){
        for (int i = 0; i < gameHeight; i++){
            Draw(gameWidth/2, i, ' ', NONE, BG_CYAN);
        }
    }

    virtual bool OnCreate(){
        paddles[0] = PongPaddle(10, 10, 4, 30);
        paddles[1] = PongPaddle(getGameWidth() - 10 - 4, 10, 4, 30);
        ball = std::make_unique<Ball>(Ball(getGameWidth()/2, getGameHeight()/2, 3));
        gameWidth = getGameWidth();
        gameHeight = getGameHeight();
    
        return true;
    }


    virtual bool GameLoop(float fDelta, char cKey){

        FillScreen();

        setAppName("Direita: " + std::to_string(scores[0]) + "   Esquerda: " + std::to_string(scores[1]));

        for (auto& p: paddles)
            p.Update(fDelta, gameHeight);
        ball->Update(fDelta, gameWidth, gameHeight, paddles);

        if (ball->CollideLeftWall(gameWidth, gameHeight)) scores[0]++;
        else if (ball->CollideRightWall(gameWidth, gameHeight)) scores[1]++;

        switch (cKey)
        {
        case 'a':
            paddles[0].ChangeDirection();
            break;
        case 'l':
            paddles[1].ChangeDirection();
            break;
        case 'q':
            return false;
            break;
        }

        for (auto& p: paddles)
            DrawPaddle(p);

        DrawLine();
        
        DrawBall(*ball);

        return true;
    }
    
};


void easyPrint(std::string string, bool endLine = true){
    std::cout << string;
    if (endLine) std::cout << std::endl;
}

int main(){

    std::string profileID = "f5f27596-afd0-420a-8aae-8220491dc05b";
    Pong engine;
    engine.ConstructConsole(400, 100, profileID, 5, "Eu amo a Ana");
    engine.Run();

}






