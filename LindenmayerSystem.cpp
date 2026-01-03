#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <unistd.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define LENGTH 5
#define FPS 60
#define RECORD_SECONDS 1.25

constexpr double ANGLE = 0.2 * M_PI;

size_t drawIndex = 0;
int MAX_BRANCHING = 8;
std::string axiom = "F";

double x, y, angle;
std::stack<std::tuple<double,double,double>> stack;

Uint32 finishedTime = 0;
const Uint32 HOLD_MS = 1000;

std::vector<std::string> branchCases = {
    "F[+F]F[-F]F",
    "F[-F]F[+F]F",
    "F[+F]F",
    "F[-F]F",
};

std::string generateAxiom(const std::string& currentAxiom) {
    std::string next;
    for (char c : currentAxiom) {
        switch (c) {
            case 'F': {
                int randCase = rand() % branchCases.size();
                next += branchCases[randCase];
                break;
            }
            default:
                next += c;
                break;
        }
    }
    return next;
}

void resetBranching() {
    MAX_BRANCHING = 5;
    drawIndex = 0;

    axiom = "F";
    for (int i = 0; i < MAX_BRANCHING; i++) {
        axiom = generateAxiom(axiom);
    }

    x = WINDOW_WIDTH / 2.0;
    y = WINDOW_HEIGHT;
    angle = -M_PI / 2.0;
    while (!stack.empty()) stack.pop();
}

void LSystem() {
    SDL_Init(SDL_INIT_VIDEO);
    srand(static_cast<unsigned>(time(nullptr)));

    SDL_Window* window = SDL_CreateWindow(
        "L-System",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    resetBranching();

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                resetBranching();
            }
        }

        if (drawIndex == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        int stepsPerFrame = 50;

        for (int i = 0; i < stepsPerFrame && drawIndex < axiom.size(); i++, drawIndex++) {
            char c = axiom[drawIndex];
            switch (c) {
                case 'F': {
                    double nx = x + LENGTH * cos(angle);
                    double ny = y + LENGTH * sin(angle);
                    SDL_RenderDrawLine(renderer, x, y, nx, ny);
                    x = nx;
                    y = ny;
                    break;
                }
                case '+': angle += ANGLE; break;
                case '-': angle -= ANGLE; break;
                case '[': stack.push({x, y, angle}); break;
                case ']':
                    if (!stack.empty()) {
                        auto [px, py, pa] = stack.top();
                        stack.pop();
                        x = px;
                        y = py;
                        angle = pa;
                    }
                    break;
            }
        }

        if (drawIndex >= axiom.size()) {
            if (finishedTime == 0) {
                finishedTime = SDL_GetTicks();
            }

            if (SDL_GetTicks() - finishedTime >= HOLD_MS) {
                finishedTime = 0;
                resetBranching();
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void LSystemFfmpeg() {
    SDL_Init(SDL_INIT_VIDEO);
    srand(static_cast<unsigned>(time(nullptr)));

    SDL_Window* window = SDL_CreateWindow(
        "L-System (FFmpeg)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    resetBranching();

    std::vector<uint8_t> pixels(WINDOW_WIDTH * WINDOW_HEIGHT * 4);

    const int total_frames = FPS * RECORD_SECONDS;
    for (int frame = 0; frame < total_frames; frame++) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return;
        }

        if (drawIndex == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        int stepsPerFrame = 50;

        for (int i = 0; i < stepsPerFrame && drawIndex < axiom.size(); i++, drawIndex++) {
            char c = axiom[drawIndex];
            switch (c) {
                case 'F': {
                    double nx = x + LENGTH * cos(angle);
                    double ny = y + LENGTH * sin(angle);
                    SDL_RenderDrawLine(renderer, x, y, nx, ny);
                    x = nx;
                    y = ny;
                    break;
                }
                case '+': angle += ANGLE; break;
                case '-': angle -= ANGLE; break;
                case '[': stack.push({x, y, angle}); break;
                case ']':
                    if (!stack.empty()) {
                        auto [px, py, pa] = stack.top();
                        stack.pop();
                        x = px;
                        y = py;
                        angle = pa;
                    }
                    break;
            }
        }

        SDL_RenderPresent(renderer);

        SDL_RenderReadPixels(
            renderer,
            nullptr,
            SDL_PIXELFORMAT_RGBA32,
            pixels.data(),
            WINDOW_WIDTH * 4
        );

        write(STDOUT_FILENO, pixels.data(), pixels.size());
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "ffmpeg") {
        LSystemFfmpeg();
    } else {
        LSystem();
    }
    return 0;
}
