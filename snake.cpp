#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

Color darkgrey = { 31, 31, 31, 255 };
Color red = { 255, 105, 105, 255 };
Color green = { 54, 255, 124, 255 };
Color darkgreen = { 36, 92, 55, 255 };
Color grey = { 125, 125, 125, 255 };
Color white = { 240, 240, 240, 255 };

int csize = 45;
int ccount = 10;
int offset = 50;
double lastUpdate = 0;
int statsOffset = 100;

bool eventTrigger(double time) {
    double current = GetTime();
    if (current - lastUpdate >= time) {
        lastUpdate = current;
        return true;
    }
    return false;
}

bool foodInS(Vector2 f, std::deque<Vector2> body) {
    for (unsigned int i = 0; i < body.size(); i++) {
        if (Vector2Equals(body[i], f)) {
            return true;
        }
    }
    return false;
}

enum GameState {
    MENU,
    RUNNING,
    PAUSED,
    GAME_OVER,
    VICTORY
};

class Snake {
public:
    std::deque<Vector2> body = { Vector2{0,0}, Vector2{0,0}, Vector2{0,0} };
    Vector2 direction = { 1,0 };

    bool change = false;
    bool add_body = false;

    Snake() {
        reset();
    }

    void draw() {
        for (unsigned int i = 0; i < body.size(); i++) {
            float t = (body.size() <= 1) ? 0.0f : (float)i / (body.size() - 1);
            Color segmentColor = ColorLerp(green, darkgreen, t);

            int x = body[i].x;
            int y = body[i].y;
            DrawRectangle(offset + x * csize, offset + y * csize, csize, csize, segmentColor);
        }
    }

    Vector2 nextHead() {
        return Vector2Add(body[0], direction);
    }

    void update() {
        body.push_front(nextHead());
        change = false;

        if (add_body) {
            add_body = false;
        }
        else {
            body.pop_back();
        }
    }

    void reset() {
        body.clear();

        int dir = GetRandomValue(0, 3);

        if (dir == 0) {
            direction = { 1,0 };
        }
        else if (dir == 1) {
            direction = { -1,0 };
        }
        else if (dir == 2) {
            direction = { 0,1 };
        }
        else {
            direction = { 0,-1 };
        }

        int x = GetRandomValue(2, ccount - 3);
        int y = GetRandomValue(2, ccount - 3);

        Vector2 head = { (float)x, (float)y };
        body.push_back(head);
        body.push_back(Vector2Subtract(head, direction));
        body.push_back(Vector2Subtract(body[1], direction));

        change = false;
        add_body = false;
    }
};

class Food {
public:
    Vector2 position;

    Food(std::deque<Vector2> body) {
        position = randomPos(body);
    }

    void draw() {
        DrawRectangle(offset + position.x * csize + csize * 0.15f,
            offset + position.y * csize + csize * 0.15f,
            csize * 0.75f,
            csize * 0.75f,
            red);
    }

    Vector2 randomCell() {
        float x = GetRandomValue(0, ccount - 1);
        float y = GetRandomValue(0, ccount - 1);
        return Vector2{ x, y };
    }

    Vector2 randomPos(std::deque<Vector2> body) {
        Vector2 pos = randomCell();
        while (foodInS(pos, body)) {
            pos = randomCell();
        }
        return pos;
    }
};

class Game {
public:
    Snake s = Snake();
    Food f = Food(s.body);
    GameState state = MENU;
    int score = 0;
    int bestScore = 0;

    void draw() {
        f.draw();
        s.draw();
    }

    void update() {
        if (state == RUNNING) {
            if (future_collision()) {
                gameover();
                return;
            }

            s.update();
            food_collision();
        }
    }

    bool future_collision() {
        Vector2 next = s.nextHead();

        if (next.x == ccount || next.x == -1) {
            return true;
        }
        if (next.y == ccount || next.y == -1) {
            return true;
        }

        std::deque<Vector2> futureBody = s.body;

        if (!s.add_body && !futureBody.empty()) {
            futureBody.pop_back();
        }

        if (foodInS(next, futureBody)) {
            return true;
        }

        return false;
    }

    void food_collision() {
        if (Vector2Equals(s.body[0], f.position)) {
            s.add_body = true;
            score++;

            if (score > bestScore) {
                bestScore = score;
            }

            if ((int)s.body.size() + 1 >= ccount * ccount) {
                state = VICTORY;
                return;
            }

            f.position = f.randomPos(s.body);
        }
    }

    void gameover() {
        state = GAME_OVER;
    }

    void start(bool newGame = false) {
        if (newGame) {
            s.reset();
            f.position = f.randomPos(s.body);
            score = 0;
        }
        state = RUNNING;
        lastUpdate = GetTime();
    }

    void pause() {
        if (state == RUNNING) {
            state = PAUSED;
        }
        else if (state == PAUSED) {
            state = RUNNING;
            lastUpdate = GetTime();
        }
    }
};

int main()
{
    std::cout << "Hello World!\n";
    InitWindow(csize * ccount + offset * 2, csize * ccount + offset * 2 + statsOffset, "snake");
    SetTargetFPS(60);

    Game g = Game();

    while (WindowShouldClose() == false)
    {
        if (IsKeyPressed(KEY_SPACE)) {
            if (g.state == MENU) {
                g.start(false);
            }
            else if (g.state == GAME_OVER || g.state == VICTORY) {
                g.start(true);
            }
            else if (g.state == RUNNING || g.state == PAUSED) {
                g.pause();
            }
        }

        if (g.state == RUNNING && !g.s.change) {
            if (IsKeyPressed(KEY_W) && g.s.direction.y != 1) {
                g.s.direction = { 0, -1 };
                g.s.change = true;
            }
            else if (IsKeyPressed(KEY_S) && g.s.direction.y != -1) {
                g.s.direction = { 0, 1 };
                g.s.change = true;
            }
            else if (IsKeyPressed(KEY_A) && g.s.direction.x != 1) {
                g.s.direction = { -1, 0 };
                g.s.change = true;
            }
            else if (IsKeyPressed(KEY_D) && g.s.direction.x != -1) {
                g.s.direction = { 1, 0 };
                g.s.change = true;
            }
        }

        if (eventTrigger(0.15)) {
            g.update();
        }

        BeginDrawing();

        ClearBackground(darkgrey);
        g.draw();

        DrawRectangleLinesEx(
            Rectangle{ (float)offset - 15, (float)offset - 15, (float)csize * ccount + 30, (float)csize * ccount + 30 },
            10,
            grey
        );

        int statsY = offset * 2 + csize * ccount - 5;

        DrawText("Score:", offset - 10, statsY, 25, white);
        DrawText(TextFormat("%i", g.score), offset + 85, statsY, 25, green);

        DrawText("Best Score:", offset - 10, statsY + 35, 25, white);
        DrawText(TextFormat("%i", g.bestScore), offset + 150, statsY + 35, 25, green);

        DrawText("Controls:", GetScreenWidth() - 255, statsY, 25, white);
        DrawText("W, A, S, D - movement", GetScreenWidth() - 255, statsY + 35, 20, grey);
        DrawText("SPACE - start/pause", GetScreenWidth() - 255, statsY + 60, 20, grey);

        if (g.state == MENU || g.state == PAUSED || g.state == GAME_OVER || g.state == VICTORY) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0, 0, 0, 170 });
        }

        if (g.state == MENU) {
            const char* text = "Press SPACE to start";
            int fontSize = 30;
            int textWidth = MeasureText(text, fontSize);

            DrawText(text,
                (GetScreenWidth() - textWidth) / 2,
                (GetScreenHeight() - fontSize) / 2,
                fontSize,
                white);
        }

        if (g.state == PAUSED) {
            const char* text1 = "Game paused";
            const char* text2 = "Press SPACE to resume";

            int fontSize1 = 40;
            int fontSize2 = 28;

            int text1Width = MeasureText(text1, fontSize1);
            int text2Width = MeasureText(text2, fontSize2);

            int centerY = GetScreenHeight() / 2;

            DrawText(text1,
                (GetScreenWidth() - text1Width) / 2,
                centerY - 30,
                fontSize1,
                white);

            DrawText(text2,
                (GetScreenWidth() - text2Width) / 2,
                centerY + 20,
                fontSize2,
                white);
        }

        if (g.state == GAME_OVER) {
            const char* text1 = "You lost!";
            const char* text2 = "Press SPACE to start";

            int fontSize1 = 40;
            int fontSize2 = 28;

            int text1Width = MeasureText(text1, fontSize1);
            int text2Width = MeasureText(text2, fontSize2);

            int centerY = GetScreenHeight() / 2;

            DrawText(text1,
                (GetScreenWidth() - text1Width) / 2,
                centerY - 30,
                fontSize1,
                red);

            DrawText(text2,
                (GetScreenWidth() - text2Width) / 2,
                centerY + 20,
                fontSize2,
                white);
        }

        if (g.state == VICTORY) {
            const char* text1 = "Victory";
            const char* text2 = "Press SPACE to restart";

            int fontSize1 = 40;
            int fontSize2 = 28;

            int text1Width = MeasureText(text1, fontSize1);
            int text2Width = MeasureText(text2, fontSize2);

            int centerY = GetScreenHeight() / 2;

            DrawText(text1,
                (GetScreenWidth() - text1Width) / 2,
                centerY - 30,
                fontSize1,
                green);

            DrawText(text2,
                (GetScreenWidth() - text2Width) / 2,
                centerY + 20,
                fontSize2,
                white);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}