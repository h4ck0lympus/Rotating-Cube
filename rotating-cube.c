#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 800.0
#define HEIGHT 600.0
#define POINT_SIZE 2
#define NUM_OF_VERTICES 8

SDL_Window* window;

typedef struct {
    double x; double y; double z;
} Point;

typedef struct {
    Point first;
    Point second;
} Line;

void drawPoint(SDL_Surface* surface, double x, double y, Uint32 color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        printf("Warning: Point (%f, %f) is outside the screen.\n", x, y);
        return;
    }
    SDL_Rect point = (SDL_Rect) { x, y, POINT_SIZE, POINT_SIZE };
    SDL_FillRect(surface, &point, color);
}

void drawLine(SDL_Surface* surface, Line line, Uint32 color)
{
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#:~:text=This%20results%20in%20an%20algorithm%20that%20uses%20only%20integer%20arithmetic.
    int x0 = (int)round(line.first.x);
    int y0 = (int)round(line.first.y);
    int x1 = (int)round(line.second.x);
    int y1 = (int)round(line.second.y);

    double dx = abs(x1 - x0);
    double sx = x0 < x1 ? 1 : -1; // directional control for x
    double dy = -abs(y1 - y0);
    double sy = y0 < y1 ? 1 : -1; // directional control for y
    double error = dx + dy;

    while (1) {
        drawPoint(surface, x0, y0, color);
        // stop drawing points when end is reached
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * error;
        if (e2 >= dy) {
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void clearScreen(SDL_Surface* surface)
{
    SDL_FillRect(surface, NULL, 0);
}

Point project(Point point) {
    // iosmeteric projection
    double scale = 100;
    Point b;
    b.x = WIDTH / 2 + (point.x - point.z) * scale;
    b.y = HEIGHT / 2 + (point.x + 2 * point.y + point.z) * scale / 2;
    b.z = 0;
    return b;
}

void drawCube(SDL_Surface* surface, Point* cube, int vertexCount, Uint32 color)
{
    for (int i=0; i < vertexCount; i++) {
        for (int j=i + 1; j < vertexCount; j++) {
            int diff = abs(i - j);
            if (diff == 1 || diff == 2 || diff == 3 || diff == 5) {
                Point p1 = project(cube[i]);
                Point p2 = project(cube[j]);
                drawLine(surface, (Line){p1, p2}, color);
            }
        }
    }
}

Point rotatePoint(Point point, double thetaX, double thetaY, double thetaZ) {
    // convert degrees to radians
    double radX = thetaX * M_PI / 180.0;
    double radY = thetaY * M_PI / 180.0;
    double radZ = thetaZ * M_PI / 180.0;

    // rotation around x axis
    double y1 = point.y * cos(radX) - point.z * sin(radX);
    double z1 = point.y * sin(radX) + point.z * cos(radX);

    // rotation around y axis
    double x2 = point.x * cos(radY) + z1 * sin(radY);
    double z2 = -point.x * sin(radY) + z1 * cos(radY);

    // rotation around z axis
    double x3 = x2 * cos(radZ) - y1 * sin(radZ);
    double y3 = x2 * sin(radZ) + y1 * cos(radZ);

    return (Point){x3, y3, z2};
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Rotating cube",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);
    if (!window) {
        perror("Failed to create window");
        SDL_Quit();
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        perror("Failed to get window surface");
        SDL_Quit();
    }

    // cube coordinates
    Point cube[8] = {
        {-1, -1, -1},
        {1, -1, -1},
        {-1, 1, -1},
        {-1, -1, 1},
        {1, 1, -1},
        {-1, 1, 1},
        {1, -1, 1},
        {1, 1, 1}
    };

    Point rotation = {0, 0, 0};

    Point rotatedCube[8];

    for (int i=0; i < NUM_OF_VERTICES; i++) {
        rotatedCube[i] = cube[i];
    }

    int quit = 0;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case (SDLK_q): quit = 1; break;
                    case (SDLK_UP): rotation.x += 5; break;
                    case (SDLK_DOWN): rotation.x -= 5; break;
                    case (SDLK_RIGHT): rotation.y += 5; break;
                    case (SDLK_LEFT): rotation.y -= 5; break;
                }
            }
        }

        for (int i=0; i < NUM_OF_VERTICES; i++) {
            rotatedCube[i] = rotatePoint(cube[i], rotation.x, rotation.y, 0);
        }

        clearScreen(surface);
        drawLine(surface, (Line){project(rotatedCube[0]), project(rotatedCube[1])},0xffffff); // difference = 1
        drawLine(surface, (Line){project(rotatedCube[1]), project(rotatedCube[4])},0xffffff); // difference = 3
        drawLine(surface, (Line){project(rotatedCube[4]), project(rotatedCube[2])},0xffffff); // difference = 2
        drawLine(surface, (Line){project(rotatedCube[2]), project(rotatedCube[0])},0xffffff); // difference = 2

        drawLine(surface, (Line){project(rotatedCube[3]), project(rotatedCube[6])},0xffffff); // difference = 3
        drawLine(surface, (Line){project(rotatedCube[6]), project(rotatedCube[7])},0xffffff); // difference = 1
        drawLine(surface, (Line){project(rotatedCube[7]), project(rotatedCube[5])},0xffffff); // difference = 2
        drawLine(surface, (Line){project(rotatedCube[5]), project(rotatedCube[3])},0xffffff); // difference = 2

        drawLine(surface, (Line){project(rotatedCube[0]), project(rotatedCube[3])},0xffffff); // difference = 3
        drawLine(surface, (Line){project(rotatedCube[1]), project(rotatedCube[6])},0xffffff); // difference = 5
        drawLine(surface, (Line){project(rotatedCube[4]), project(rotatedCube[7])},0xffffff); // difference = 3
        drawLine(surface, (Line){project(rotatedCube[2]), project(rotatedCube[5])},0xffffff); // difference = 3
        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);
    }

    SDL_Quit();
    SDL_DestroyWindow(window);
    return 0;
}
