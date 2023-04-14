#include "ASCIIEngine.h"
#include <fstream>
#include <string>
#include <random>

class PongPaddle {

    float m_x{}, m_y{};
    float m_direction = 1.f;
    std::string m_base;
    float m_height{}, m_width{}, m_speed{};
    
    public:
    PongPaddle() = default;

    PongPaddle(float x, float y, float width, float height, float speed):
    m_x(x), m_y(y), m_height(height), m_width(width), m_speed(speed) {
        char base[int(width) + 1];
        for (int i = 0; i < int(width); i ++){
            base[i] = ' ';
        }
        base[int(width)] = '\0';
        m_base = base;
    }

    void ChangeDirection(){
        m_direction *= -1;
    }
    
    void Update(float fDelta, int gameHeight){
        m_y += m_speed*fDelta*m_direction;
        if (m_y < 0) {
            m_y = 0;
            ChangeDirection();
        } else if (m_y + m_height > float(gameHeight)){
            m_y = (float) gameHeight - m_height;
            ChangeDirection();
        } 
    }

    [[nodiscard]] float getX() const { return m_x; }
    [[nodiscard]] float getY() const { return m_y; }
    [[nodiscard]] int getIntX() const { return int(m_x); }
    [[nodiscard]] int getIntY() const { return int(m_y); }
    [[nodiscard]] float getHeight() const { return m_height; }
    [[nodiscard]] float getWidth() const { return m_width; }
    [[nodiscard]] int getIntHeight() const { return int(m_height); }
    [[nodiscard]] int getIntWidth() const { return int(m_width); }
    [[nodiscard]] std::string getBase() const { return m_base; }


};


class Ball {

    float m_x{}, m_y{};
    float m_dir[2]{};
    float m_radius, m_speed;
    float m_initialSpeed, m_maxSpeed, m_acceleration;

public:

    Ball(int gameWidth, int gameHeight, float radius, float initialSpeed, float maxSpeed, float accelerationTime):
    m_radius(radius), m_speed(initialSpeed), m_initialSpeed(initialSpeed), m_maxSpeed(maxSpeed), m_acceleration((maxSpeed - initialSpeed) / accelerationTime) {
        ResetBall(gameWidth, gameHeight);
    }

    void BounceX() { m_dir[0] *= -1; }
    void BounceY() { m_dir[1] *= -1; }

    void CollideUpperWalls(int gameHeight){
        if (m_y - m_radius <= 0) {m_y = m_radius; BounceY();} 
        else if (m_y + m_radius >= float(gameHeight)) {m_y = float(gameHeight) - m_radius; BounceY();}
    }

    void ResetBall(int gameWidth, int gameHeight){
        m_speed = m_initialSpeed;
        m_x = float(gameWidth)/2;
        m_y = float(gameHeight)/2;

        static std::random_device rd; // obtain a random number from hardware
        static std::mt19937 gen(rd()); // seed the generator
        static std::uniform_real_distribution<> distr(-0.7, 0.7);
        m_dir[1] = (float) distr(gen);
        m_dir[0] = distr(gen) < 0 ? -std::sqrt(1-m_dir[1]*m_dir[1]): std::sqrt(1-m_dir[1]*m_dir[1]);
    }

    bool CollideLeftWall(int gameWidth, int gameHeight){
        if (m_x - m_radius <= 0) {m_x = m_radius; ResetBall(gameWidth, gameHeight); return true;}
        return false; 
    }

    bool CollideRightWall(int gameWidth, int gameHeight){
        if (m_x + m_radius >= float(gameWidth)) {m_x = float(gameWidth) - m_radius; ResetBall(gameWidth, gameHeight); return true; }
        return false;
    }

    void CollidePaddleRight(const PongPaddle& paddle, bool xCollision){
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

    void CollidePaddleLeft(const PongPaddle& paddle, bool xCollision){
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

    void Update(float fDelta, int gameHeight, PongPaddle paddles[2]) {
        m_speed = std::min(m_maxSpeed, m_speed + fDelta*m_acceleration);
        m_x += m_speed*fDelta*m_dir[0];
        CollidePaddleLeft(paddles[0], true);
        CollidePaddleRight(paddles[1], true);

        m_y += m_speed*fDelta*m_dir[1];
        CollidePaddleLeft(paddles[0], false);
        CollidePaddleRight(paddles[1], false);

        CollideUpperWalls(gameHeight);
    }


    [[nodiscard]] int getX() const { return int(m_x); }
    [[nodiscard]] int getY() const { return int(m_y); }
    [[nodiscard]] int getRadius() const { return int(m_radius); }
};


class Pong: public aen::ASCIIEngine {

    PongPaddle paddles[2] = {PongPaddle(), PongPaddle()};
    int scores[2] = {0, 0}, gameWidth{}, gameHeight{};
    std::unique_ptr<Ball> ball;
    bool scoreChanged = false;
    float time{}, ballAccelerationTime = 15.f;

protected:
    void DrawPaddle(const PongPaddle& paddle){
        for (int i = 0; i < paddle.getIntHeight(); i++)
            DrawString(paddle.getIntX(), paddle.getIntY() + i, paddle.getBase(), NONE, BG_RED);
    }

    void DrawBall(){
        int yc = ball->getY(), xc = ball->getX(), r = ball->getRadius();
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

    bool OnCreate() override{
        auto fWidth = float(getGameWidth());
        auto fHeight = float(getGameHeight());
        float paddleHeight = std::max(fHeight/3, 1.f);
        float paddleWidth = std::max(fWidth/100, 1.f);
        float paddleX = fWidth/40;
        float paddleY = fHeight/10;
        float paddleSpeed = 60*fHeight/100;
        paddles[0] = PongPaddle(paddleX, paddleY, paddleWidth, paddleHeight, paddleSpeed);
        paddles[1] = PongPaddle(fWidth - paddleX, paddleY, paddleWidth, paddleHeight, paddleSpeed);
        ball = std::make_unique<Ball>(getGameWidth()/2, getGameHeight()/2,
                                      std::max(fWidth/100, 1.f), fWidth/3.5f,
                                      fWidth/2, ballAccelerationTime);
        gameWidth = getGameWidth();
        gameHeight = getGameHeight();
        setAppName("Direita: " + std::to_string(scores[0]) + "   Esquerda: " + std::to_string(scores[1]));

        return true;
    }


    bool GameLoop(float fDelta, char cKey) override{
        time += fDelta;

        FillScreen();

        if (scoreChanged) {
            setAppName("Direita: " + std::to_string(scores[0]) + "   Esquerda: " + std::to_string(scores[1]));
            scoreChanged = false;
        }

        for (auto& p: paddles)
            p.Update(fDelta, gameHeight);
        ball->Update(fDelta, gameHeight, paddles);

        if (ball->CollideLeftWall(gameWidth, gameHeight)) {
            scores[0]++;
            scoreChanged = true;
        }
        else if (ball->CollideRightWall(gameWidth, gameHeight)) {
            scores[1]++;
            scoreChanged = true;
        }

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
        default:
            break;
        }

        for (auto& p: paddles)
            DrawPaddle(p);

        DrawLine();
        
        DrawBall();

        return true;
    }
    
};


void easyPrint(const std::string& string, bool endLine = true){
    std::cout << string;
    if (endLine) std::cout << std::endl;
}


int main(){
//    std::string profileID = "f5f27596-afd0-420a-8aae-8220491dc05b";
    Pong engine;
    engine.ConstructConsole(600, 150, "");
    engine.Run();
}






