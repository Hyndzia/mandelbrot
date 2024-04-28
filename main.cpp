#include <iostream>
#include <SDL2/SDL.h>
#include <omp.h>


const int WIDTH = 800;
const int HEIGHT = 600;
double MIN_X = -2.0;
double MAX_X = 1.0;
double MIN_Y = -1.5;
double MAX_Y = 1.5;
const int MAX_ITERATIONS = 2000;

// Function to check if a point (x, y) is in the Mandelbrot set
int mandelbrot(double x, double y) {
    double zx = 0.0;
    double zy = 0.0;
    int iteration = 0;

    while (zx * zx + zy * zy < 4.0 && iteration < MAX_ITERATIONS) {
        double temp = zx * zx - zy * zy + x;
        zy = 2.0 * zx * zy + y;
        zx = temp;
        iteration++;
    }

    if (iteration == MAX_ITERATIONS) {
        return 0; // Point belongs to the Mandelbrot set
    }
    else {
        return iteration; // Return the number of iterations taken to escape
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (texture == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    Uint32* pixels = new Uint32[WIDTH * HEIGHT];

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    double mouseX = MIN_X + (MAX_X - MIN_X) * event.button.x / WIDTH;
                    double mouseY = MIN_Y + (MAX_Y - MIN_Y) * event.button.y / HEIGHT;
                    double rangeX = MAX_X - MIN_X;
                    double rangeY = MAX_Y - MIN_Y;
                    MIN_X = mouseX - rangeX / 4;
                    MAX_X = mouseX + rangeX / 4;
                    MIN_Y = mouseY - rangeY / 4;
                    MAX_Y = mouseY + rangeY / 4;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    double mouseX = static_cast<double>(event.button.x) / WIDTH;
                    double mouseY = static_cast<double>(event.button.y) / HEIGHT;
                    double rangeX = MAX_X - MIN_X;
                    double rangeY = MAX_Y - MIN_Y;
                    MIN_X = mouseX - rangeX * 4;
                    MAX_X = mouseX + rangeX * 4;
                    MIN_Y = mouseY - rangeY * 4;
                    MAX_Y = mouseY + rangeY * 4;
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_s) {
                    MIN_X = -2.0;
                    MAX_X = 1.0;
                    MIN_Y = -1.5;
                    MAX_Y = 1.5;
                }
                break;
            }
        }

#pragma omp parallel for collapse(2)
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                double real = MIN_X + (MAX_X - MIN_X) * x / WIDTH;
                double imag = MIN_Y + (MAX_Y - MIN_Y) * y / HEIGHT;

                int color = mandelbrot(real, imag);
                Uint8 r = 0;
                Uint8 g = color % 256;
                Uint8 b = (color * color) % 256;

                pixels[y * WIDTH + x] = (0 << 24) | (r << 12) | (g << 11) | b;
            }
        }

        SDL_UpdateTexture(texture, nullptr, pixels, WIDTH * sizeof(Uint32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
