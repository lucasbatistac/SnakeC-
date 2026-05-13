#include <iostream>
#include <deque>
#include <vector>
#include <optional>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <sstream>
#include <conio.h>
#include <windows.h>

using namespace std;

struct Point {
    int x;
    int y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

class SnakeGame {
private:
    const int width = 42;
    const int height = 22;
    const string highScoreFile = "recorde_snake.txt";

    deque<Point> snake;
    vector<Point> obstacles;

    Point food{-1, -1};
    optional<Point> bonus;

    Direction dir = Direction::Right;
    Direction nextDir = Direction::Right;

    bool gameOver = false;
    bool paused = false;

    int score = 0;
    int highScore = 0;
    int difficulty = 1;
    int delayMs = 120;
    int bonusLife = 0;
    int tick = 0;

    mt19937 rng;

public:
    SnakeGame() : rng(random_device{}()) {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleTitleA("Snake C++ - Console Game");
        highScore = loadHighScore();
        hideCursor();
    }

    void run() {
        while (true) {
            system("cls");

            cout << "=========================================\n";
            cout << "           SNAKE C++ - CONSOLE\n";
            cout << "=========================================\n\n";

            cout << "Recorde atual: " << highScore << "\n\n";

            cout << "[1] Novo jogo\n";
            cout << "[2] Controles\n";
            cout << "[3] Sair\n\n";
            cout << "Escolha: ";

            char option = static_cast<char>(_getch());

            if (option == '1') {
                if (chooseDifficulty()) {
                    startGame();
                }
            } else if (option == '2') {
                showControls();
            } else if (option == '3') {
                system("cls");
                return;
            }
        }
    }

private:
    int loadHighScore() {
        ifstream file(highScoreFile);
        int value = 0;

        if (file >> value) {
            return value;
        }

        return 0;
    }

    void saveHighScore() {
        ofstream file(highScoreFile);
        file << highScore;
    }

    void hideCursor() {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;

        GetConsoleCursorInfo(console, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(console, &cursorInfo);
    }

    void setCursorPosition(int x, int y) {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD pos;
        pos.X = static_cast<SHORT>(x);
        pos.Y = static_cast<SHORT>(y);
        SetConsoleCursorPosition(console, pos);
    }

    bool chooseDifficulty() {
        while (true) {
            system("cls");

            cout << "============ DIFICULDADE ============\n\n";
            cout << "[1] Fácil   - sem obstáculos\n";
            cout << "[2] Médio   - obstáculos moderados\n";
            cout << "[3] Difícil - mais obstáculos e velocidade\n";
            cout << "[ESC] Voltar\n\n";
            cout << "Escolha: ";

            int option = _getch();

            if (option == 27) {
                return false;
            }

            if (option == '1') {
                difficulty = 1;
                delayMs = 120;
                return true;
            }

            if (option == '2') {
                difficulty = 2;
                delayMs = 85;
                return true;
            }

            if (option == '3') {
                difficulty = 3;
                delayMs = 55;
                return true;
            }
        }
    }

    void showControls() {
        system("cls");

        cout << "================ CONTROLES ================\n\n";

        cout << "Mover para cima:     W ou seta para cima\n";
        cout << "Mover para baixo:    S ou seta para baixo\n";
        cout << "Mover para esquerda: A ou seta para esquerda\n";
        cout << "Mover para direita:  D ou seta para direita\n";
        cout << "Pausar/continuar:    P\n";
        cout << "Sair da partida:     ESC\n\n";

        cout << "Símbolos do jogo:\n";
        cout << "@  Cabeça da cobra\n";
        cout << "o  Corpo da cobra\n";
        cout << "*  Comida normal\n";
        cout << "$  Bônus temporário\n";
        cout << "X  Obstáculo\n";
        cout << "#  Parede\n\n";

        cout << "Pressione qualquer tecla para voltar...";
        _getch();
    }

    void startGame() {
        initializeGame();

        system("cls");

        while (!gameOver) {
            auto frameStart = chrono::steady_clock::now();

            handleInput();

            if (!paused) {
                updateGame();
            }

            draw();

            this_thread::sleep_until(frameStart + chrono::milliseconds(delayMs));
        }

        showGameOverScreen();
    }

    void initializeGame() {
        snake.clear();
        obstacles.clear();
        bonus.reset();

        score = 0;
        tick = 0;
        bonusLife = 0;

        gameOver = false;
        paused = false;

        dir = Direction::Right;
        nextDir = Direction::Right;

        int centerX = width / 2;
        int centerY = height / 2;

        snake.push_back({centerX, centerY});
        snake.push_back({centerX - 1, centerY});
        snake.push_back({centerX - 2, centerY});

        generateObstacles();
        spawnFood();
    }

    void generateObstacles() {
        int amount = 0;

        if (difficulty == 2) {
            amount = 10;
        } else if (difficulty == 3) {
            amount = 18;
        }

        for (int i = 0; i < amount; i++) {
            obstacles.push_back(randomFreeCell());
        }
    }

    Point randomFreeCell() {
        uniform_int_distribution<int> distX(1, width - 2);
        uniform_int_distribution<int> distY(1, height - 2);

        while (true) {
            Point p{distX(rng), distY(rng)};

            if (!isSnakeCell(p) &&
                !isObstacleCell(p) &&
                !(p == food) &&
                !(bonus.has_value() && p == bonus.value())) {
                return p;
            }
        }
    }

    void spawnFood() {
        food = randomFreeCell();
    }

    void spawnBonus() {
        if (!bonus.has_value()) {
            bonus = randomFreeCell();
            bonusLife = 70;
        }
    }

    bool isSnakeCell(const Point& p) const {
        for (const auto& part : snake) {
            if (part == p) {
                return true;
            }
        }

        return false;
    }

    bool isSnakeCollision(const Point& p, bool ignoreTail) const {
        size_t limit = snake.size();

        if (ignoreTail && limit > 0) {
            limit--;
        }

        for (size_t i = 0; i < limit; i++) {
            if (snake[i] == p) {
                return true;
            }
        }

        return false;
    }

    bool isObstacleCell(const Point& p) const {
        for (const auto& obstacle : obstacles) {
            if (obstacle == p) {
                return true;
            }
        }

        return false;
    }

    bool isWallCollision(const Point& p) const {
        return p.x <= 0 || p.x >= width - 1 || p.y <= 0 || p.y >= height - 1;
    }

    bool isOpposite(Direction a, Direction b) const {
        return (a == Direction::Up && b == Direction::Down) ||
               (a == Direction::Down && b == Direction::Up) ||
               (a == Direction::Left && b == Direction::Right) ||
               (a == Direction::Right && b == Direction::Left);
    }

    void setDirection(Direction newDir) {
        if (!isOpposite(newDir, dir)) {
            nextDir = newDir;
        }
    }

    void handleInput() {
        while (_kbhit()) {
            int key = _getch();

            if (key == 0 || key == 224) {
                int arrow = _getch();

                if (arrow == 72) setDirection(Direction::Up);
                if (arrow == 80) setDirection(Direction::Down);
                if (arrow == 75) setDirection(Direction::Left);
                if (arrow == 77) setDirection(Direction::Right);

                continue;
            }

            char c = static_cast<char>(tolower(key));

            if (c == 'w') setDirection(Direction::Up);
            if (c == 's') setDirection(Direction::Down);
            if (c == 'a') setDirection(Direction::Left);
            if (c == 'd') setDirection(Direction::Right);

            if (c == 'p') {
                paused = !paused;
            }

            if (key == 27) {
                gameOver = true;
            }
        }
    }

    Point getNextHead() const {
        Point head = snake.front();

        if (nextDir == Direction::Up) {
            head.y--;
        } else if (nextDir == Direction::Down) {
            head.y++;
        } else if (nextDir == Direction::Left) {
            head.x--;
        } else if (nextDir == Direction::Right) {
            head.x++;
        }

        return head;
    }

    void updateGame() {
        tick++;

        dir = nextDir;
        Point newHead = getNextHead();

        bool willEatFood = newHead == food;
        bool willEatBonus = bonus.has_value() && newHead == bonus.value();

        if (isWallCollision(newHead) ||
            isObstacleCell(newHead) ||
            isSnakeCollision(newHead, !willEatFood)) {
            gameOver = true;
            return;
        }

        snake.push_front(newHead);

        if (willEatFood) {
            score += 10 * difficulty;
            spawnFood();
        } else {
            snake.pop_back();
        }

        if (willEatBonus) {
            score += 50 * difficulty;
            bonus.reset();
            bonusLife = 0;
        }

        if (bonus.has_value()) {
            bonusLife--;

            if (bonusLife <= 0) {
                bonus.reset();
            }
        }

        if (tick % 45 == 0) {
            spawnBonus();
        }

        if (score > highScore) {
            highScore = score;
        }
    }

    char getCellChar(int x, int y) const {
        Point p{x, y};

        if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
            return '#';
        }

        if (!snake.empty() && snake.front() == p) {
            return '@';
        }

        for (size_t i = 1; i < snake.size(); i++) {
            if (snake[i] == p) {
                return 'o';
            }
        }

        if (p == food) {
            return '*';
        }

        if (bonus.has_value() && p == bonus.value()) {
            return '$';
        }

        if (isObstacleCell(p)) {
            return 'X';
        }

        return ' ';
    }

    void draw() {
        setCursorPosition(0, 0);

        ostringstream screen;

        screen << "SCORE: " << score
               << " | RECORDE: " << highScore
               << " | DIFICULDADE: " << difficulty
               << " | P: Pausar"
               << " | ESC: Sair\n";

        if (paused) {
            screen << "STATUS: PAUSADO\n";
        } else {
            screen << "STATUS: JOGANDO\n";
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                screen << getCellChar(x, y);
            }
            screen << '\n';
        }

        if (bonus.has_value()) {
            screen << "Bônus ativo por mais " << bonusLife << " ciclos.\n";
        } else {
            screen << "Bônus: indisponível no momento.\n";
        }

        cout << screen.str() << flush;
    }

    void showGameOverScreen() {
        saveHighScore();

        setCursorPosition(0, height + 5);

        cout << "\n=========================================\n";
        cout << "               GAME OVER\n";
        cout << "=========================================\n";
        cout << "Pontuação final: " << score << "\n";
        cout << "Recorde atual:   " << highScore << "\n\n";
        cout << "Pressione qualquer tecla para voltar ao menu...";

        _getch();
    }
};

int main() {
    SnakeGame game;
    game.run();

    return 0;
}