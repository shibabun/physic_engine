/*
./physic_engine.exe
C:/tekito/C++/mingw64/bin/g++.exe physic_engine.cpp -o physic_engine.exe -I "C:/download_libraries/SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit/SFML-3.0.2/include" -L "C:/download_libraries/SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit/SFML-3.0.2/lib" -lsfml-window -lsfml-system -lsfml-graphics
*/
/*
1. ボールが跳ね回るやつを作る
*/
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

float randomFloat(float min, float max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

struct wall{
    float x1, y1, x2, y2; // 壁の端点座標
};

class vector2{
    public:
        float x, y;
        vector2(float x = 0, float y = 0) : x(x), y(y) {}
        vector2 operator+(const vector2& other) const {
            return vector2{x + other.x, y + other.y};
        }
        vector2 operator-(const vector2& other) const {
            return vector2{x - other.x, y - other.y};
        }
        vector2 operator*(float scalar) const {
            return vector2{x * scalar, y * scalar};
        }
        float length() const {
            return sqrt(x * x + y * y);
        }
        float dot(const vector2& other) const {
            return x * other.x + y * other.y;
        }
        vector2 rotate(float angle) const {
            float rad = angle * 3.14159265f / 180.f;
            float cosA = cos(rad);
            float sinA = sin(rad);
            return vector2{
                x * cosA - y * sinA,
                x * sinA + y * cosA
            };
        }
        vector2 normalized() const {
            float len = length();
            if (len > 0) {
                return vector2{x / len, y / len};
            }
            return vector2{0, 0};
        }
};

struct ball{
    vector2 position;
    vector2 velocity;
    float radius;

    vector2 isCollidedWithWall(const wall& w) const {
        vector2 wallDir{w.x2 - w.x1, w.y2 - w.y1};
        vector2 wallNormal{-wallDir.y, wallDir.x}; // 法線ベクトル
        float length = wallNormal.length();
        if (length > 0) {
            wallNormal.x /= length;
            wallNormal.y /= length;
        }

        vector2 ballToWallStart{position.x - w.x1, position.y - w.y1};
        float distance = ballToWallStart.dot(wallNormal);

        if (distance < radius) {
            return wallNormal; // 衝突している場合は法線ベクトルを返す
        }
        return vector2{0, 0}; // 衝突していない場合はゼロベクトルを返す
    }

    vector2 isCollidedWithBall(const ball& other) const {
        vector2 ballToBall = other.position - position;
        float distance = ballToBall.length();
        if (distance < radius + other.radius) {
            return ballToBall.normalized(); // 衝突している場合は法線ベクトルを返す
        }
        return vector2{0, 0}; // 衝突していない場合はゼロベクトルを返す
    }

    vector2 reflect(const vector2& normal) const {
        float velocityDotNormal = velocity.dot(normal);
        return velocity - normal * (2 * velocityDotNormal);
    }

    vector2 resolveCollision(const wall& w) {
        vector2 normal = isCollidedWithWall(w);
        if (normal.x != 0 || normal.y != 0) {
            return reflect(normal); // 衝突している場合は反射ベクトルを返す
        }
        return velocity; // 衝突していない場合は元の速度を返す
    }


};

class PhysicEngineBall {
    public:
        vector<wall> walls; // 壁のリスト
        vector<ball> balls; // ボールのリスト
        int matrix_height = 60; // 衝突行列の高さ
        int matrix_width = 80; // 衝突行列の幅
        vector<vector<vector<int>>> collisionMatrix; // 衝突行列

        PhysicEngineBall() {
            collisionMatrix.resize(matrix_height, vector<vector<int>>(matrix_width));
        }

        void addRectangleWall(float x, float y, float width, float height, float angle) {
            vector2 A = vector2{width / 2, height / 2}.rotate(angle) + vector2{x, y};
            vector2 B = vector2{-width / 2, height / 2}.rotate(angle) + vector2{x, y};
            vector2 C = vector2{-width / 2, -height / 2}.rotate(angle) + vector2{x, y};
            vector2 D = vector2{width / 2, -height / 2}.rotate(angle) + vector2{x, y};
            walls.push_back({A.x, A.y, B.x, B.y});
            walls.push_back({B.x, B.y, C.x, C.y});
            walls.push_back({C.x, C.y, D.x, D.y});
            walls.push_back({D.x, D.y, A.x, A.y});
        }

        void addBall(float x, float y, float vx, float vy, float radius) {
            balls.push_back({{x, y}, {vx, vy}, radius});
        }

        pair<int, int> getCollisionMatrixIndex(const ball& b) const {
            int xIndex = static_cast<int>(b.position.x / (matrix_width * 2 * b.radius));
            int yIndex = static_cast<int>(b.position.y / (matrix_height * 2 * b.radius));
            return {xIndex, yIndex};
        }

        void updateCollisionMatrix() {
            for (auto& row : collisionMatrix) {
                for (auto& cell : row) {
                    cell.clear();
                }
            }
            for (size_t i = 0; i < balls.size(); ++i) {
                auto [xIndex, yIndex] = getCollisionMatrixIndex(balls[i]);
                if (xIndex >= 0 && xIndex < matrix_width && yIndex >= 0 && yIndex < matrix_height) {
                    collisionMatrix[yIndex][xIndex].push_back(i);
                }
            }
        }

        void deleteWalls() {
            walls.clear();
        }

        void update(float dt) {
            for (ball& b : balls) {
                // 位置の更新
                b.position = b.position + b.velocity * dt;
                // 壁との衝突処理
                for (const wall& w : walls) {
                    b.velocity = b.resolveCollision(w);
                }

                for(size_t i = 0; i < balls.size(); ++i) {
                    auto[xIndex, yIndex] = getCollisionMatrixIndex(balls[i]);
                    if(xIndex >= 0 && xIndex < matrix_width && yIndex >= 0 && yIndex < matrix_height) {
                        for(int j = -1; j <= 1; ++j) {
                            for(int k = -1; k <= 1; ++k) {
                                int neighborX = xIndex + j;
                                int neighborY = yIndex + k;
                                if(neighborX >= 0 && neighborX < matrix_width && neighborY >= 0 && neighborY < matrix_height) {
                                    for(int otherIndex : collisionMatrix[neighborY][neighborX]) {
                                        if(otherIndex != i) { // 自分自身との衝突は無視
                                            vector2 normal = balls[i].isCollidedWithBall(balls[otherIndex]);
                                            if (normal.x != 0 || normal.y != 0) {
                                                balls[i].velocity = balls[i].reflect(normal); // 衝突している場合は反射ベクトルを返す
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
};

int main() {
    PhysicEngineBall PhysicEngineBall;
    const float width = 800.f;
    const float height = 600.f;
    const float dt = 0.001f;
    const float deltaAngle = 1.f; // 壁の回転速度
    float angle = 0.f; // 壁の回転角度
    PhysicEngineBall.addRectangleWall(width / 2, height / 2, width, height, angle); // 画面全体を覆う壁を追加

    sf::RenderWindow window(sf::VideoMode({(unsigned int)width, (unsigned int)height}), "Physics Engine");

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::L){
                    // Lキーでボールを追加
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    float initialVx = randomFloat(-200.f, 200.f); // ランダムな初速度X
                    float initialVy = randomFloat(-200.f, 200.f); // ランダムな初速度Y
                    PhysicEngineBall.addBall(mousePos.x, mousePos.y, initialVx, initialVy, 10.f);
                }
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){
            angle += deltaAngle; // Aキーで壁を回転
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)){
            angle -= deltaAngle; // Dキーで壁を回転
        }
        PhysicEngineBall.deleteWalls();
        PhysicEngineBall.addRectangleWall(width / 2, height / 2, width, height, angle); // 回転した壁を再追加
        PhysicEngineBall.update(dt); // 60FPSを想定

        window.clear();
        // ボールの描画
        for (const ball& b : PhysicEngineBall.balls) {
            sf::CircleShape shape(b.radius);
            sf::Vector2f position(b.position.x, b.position.y);
            shape.setPosition(position); // 中心を位置に合わせる
            shape.setFillColor(sf::Color::Green);
            window.draw(shape);
        }
        for (const wall& w : PhysicEngineBall.walls) {
            sf::Vertex line[] = {
                sf::Vertex{sf::Vector2f(w.x1, w.y1)},
                sf::Vertex{sf::Vector2f(w.x2, w.y2)}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
        window.display();
    }
    return 0;
}