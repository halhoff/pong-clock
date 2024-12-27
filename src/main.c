#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "util/types.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200
#define PADDLE_SPEED 10
#define INIT_BALL_SPEED 5
#define BALL_SPEED_MULT 1.05
#define BALL_SPEED_CAP 15
#define BALL_SIZE 20
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 100
#define TICK 10 // milliseconds
#define TRAIL_LENGTH 50

struct Paddle {
    f32 x, y;
    f32 dy;
};

struct Ball {
    f32 x, y;
    f32 dx, dy;
};

struct Trajectory {
    bool collision; // true is left paddle, false is right paddle
    f32 y;
};

void initPaddle(struct Paddle *paddle, f32 x, f32 y) {
    paddle->x = x;
    paddle->y = y;
    paddle->dy = PADDLE_SPEED;
}

void initBall(struct Ball *ball) {
    ball->x = SCREEN_WIDTH / 2;
    ball->y = SCREEN_HEIGHT / 2;
    ball->dx = -INIT_BALL_SPEED;
    ball->dy = -INIT_BALL_SPEED;
}

void drawRectangle(f32 x, f32 y, f32 width, f32 height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawDigit(f32 x, f32 y, int digit) {
    //  _  1 
    // | | 2 3
    //  —  4
    // | | 5 6
    //  —  7
    static const bool SEGMENTS[10][7]= {
        {1,1,1,0,1,1,1},
        {0,0,1,0,0,1,0},
        {1,0,1,1,1,0,1},
        {1,0,1,1,0,1,1},
        {0,1,1,1,0,1,0},
        {1,1,0,1,0,1,1},
        {1,1,0,1,1,1,1},
        {1,0,1,0,0,1,0},
        {1,1,1,1,1,1,1},
        {1,1,1,1,0,1,1}
    };
    static const int LOCATIONS[7][4] = {
        {10,0,100,10},
        {0,10,10,100},
        {110,10,10,100},
        {10,110,100,10},
        {0,120,10,100},
        {110,120,10,100},
        {10,220,100,10}
    };
    for (int i = 0; i < 7; ++i) {
        if (SEGMENTS[digit][i]) {
            drawRectangle(x + LOCATIONS[i][0], y + LOCATIONS[i][1], LOCATIONS[i][2], LOCATIONS[i][3]);
        }
    }
}

void displayTime(void) {
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    int hours = local_time->tm_hour;
    int minutes = local_time->tm_min;
    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
    drawDigit(SCREEN_WIDTH / 2 - 280, 10, hours / 10);
    drawDigit(SCREEN_WIDTH / 2 - 140, 10, hours % 10);
    drawDigit(SCREEN_WIDTH / 2 + 20, 10, minutes / 10);
    drawDigit(SCREEN_WIDTH / 2 + 160, 10, minutes % 10);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void drawColon(void) {
    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
    drawRectangle(SCREEN_WIDTH / 2 - 5, 60, 10, 10);
    drawRectangle(SCREEN_WIDTH / 2 - 5, 180, 10, 10);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void calculateTrajectory(struct Trajectory *trajectory, struct Ball *ball, struct Paddle *leftPaddle, struct Paddle *rightPaddle) {
    trajectory->collision = ball->dx < 0;
    struct Ball ballCopy = *ball;
    ballCopy.x += ballCopy.dx;
    ballCopy.y += ballCopy.dy;
    f32 distanceX = trajectory->collision ? leftPaddle->x - ballCopy.x : rightPaddle->x - ballCopy.x - BALL_SIZE;
    u8 tickCount = (u8)(distanceX / ballCopy.dx);
    if (tickCount < 1) {
        tickCount = 1;
    }

    while (ballCopy.x > leftPaddle->x + PADDLE_WIDTH && ballCopy.x + BALL_SIZE < rightPaddle->x) {
        ballCopy.x += ballCopy.dx;
        ballCopy.y += ballCopy.dy;

        if (ballCopy.y <= 0 || ballCopy.y + BALL_SIZE >= SCREEN_HEIGHT) {
            ballCopy.dy *= -1;
        }
    }
    trajectory->y = ballCopy.y - PADDLE_HEIGHT / 2;
    if (trajectory->y >= SCREEN_HEIGHT - PADDLE_HEIGHT) {
        trajectory->y = SCREEN_HEIGHT - PADDLE_HEIGHT;
    }
    else if (trajectory->y < PADDLE_HEIGHT / 2) {
        trajectory->y = 0;
    }

    if (trajectory->collision) {
        leftPaddle->dy = - (leftPaddle->y - trajectory->y) / tickCount;
    }
    else {
        rightPaddle->dy = - (rightPaddle->y - trajectory->y) / tickCount;
    }
}

void updatePaddle(struct Paddle *leftPaddle, struct Paddle *rightPaddle, struct Trajectory *trajectory) {
    if (trajectory->collision) {
        leftPaddle->y += leftPaddle->dy;
    }
    else {
        rightPaddle->y += rightPaddle->dy;
    }
}

void updateBall(struct Ball *ball, struct Paddle *leftPaddle, struct Paddle *rightPaddle, struct Trajectory *trajectory) {
    ball->x += ball->dx;
    ball->y += ball->dy;

    // vertical borders
    if (ball->y <= 0 || ball->y + BALL_SIZE >= SCREEN_HEIGHT) {
        ball->dy *= -1;
    }

    // left paddle collision
    if (ball->x <= leftPaddle->x + PADDLE_WIDTH && ball->y + BALL_SIZE >= leftPaddle->y && ball->y <= leftPaddle->y + PADDLE_HEIGHT && ball->dx < 0) {
        if (ball->y + BALL_SIZE > leftPaddle->y && ball->y < leftPaddle->y + PADDLE_HEIGHT) {
            ball->dx *= -1;
            ball->x = leftPaddle->x + PADDLE_WIDTH;
        }
        ball->dx *= BALL_SPEED_MULT;
        if (ball->dx > BALL_SPEED_CAP) {
            ball->dx = BALL_SPEED_CAP;
        }
        calculateTrajectory(trajectory, ball, leftPaddle, rightPaddle);
    }

    // right paddle collision
    if (ball->x + BALL_SIZE >= rightPaddle->x && ball->y + BALL_SIZE >= rightPaddle->y && ball->y <= rightPaddle->y + PADDLE_HEIGHT && ball->dx > 0) {
        if (ball->y + BALL_SIZE > rightPaddle->y && ball->y < rightPaddle->y + PADDLE_HEIGHT) {
            ball->dx *= -1;
            ball->x = rightPaddle->x - BALL_SIZE;
        }
        ball->dx *= BALL_SPEED_MULT;
        if (ball->dx > BALL_SPEED_CAP) {
            ball->dx = BALL_SPEED_CAP;
        }
        calculateTrajectory(trajectory, ball, leftPaddle, rightPaddle);
    }

    // horizontal borders
    if (ball->x <= 0 || ball->x + BALL_SIZE >= SCREEN_WIDTH) {
        ball->x = SCREEN_WIDTH / 2;
        ball->y = SCREEN_HEIGHT / 2;
        ball->dx = INIT_BALL_SPEED;
        ball->dx *= -1;
        calculateTrajectory(trajectory, ball, leftPaddle, rightPaddle);
    }
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glOrtho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f);

    struct Paddle leftPaddle, rightPaddle;
    struct Ball ball;
    struct Trajectory trajectory;

    initPaddle(&leftPaddle, 50, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2);
    initPaddle(&rightPaddle, SCREEN_WIDTH - 50 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2);
    initBall(&ball);
    calculateTrajectory(&trajectory, &ball, &leftPaddle, &rightPaddle);

    f64 start_time = glfwGetTime();
    f64 current_time;
    f64 elapsed_time;

    while (!glfwWindowShouldClose(window)) {
        current_time = glfwGetTime();
        elapsed_time = (current_time - start_time) * 1000.0;
        if (elapsed_time >= TICK) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawColon();
            updateBall(&ball, &leftPaddle, &rightPaddle, &trajectory);
            updatePaddle(&leftPaddle, &rightPaddle, &trajectory);
            drawRectangle(leftPaddle.x, leftPaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT);
            drawRectangle(rightPaddle.x, rightPaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT);
            drawRectangle(ball.x, ball.y, BALL_SIZE, BALL_SIZE);
            displayTime();
            glfwSwapBuffers(window);
            glfwPollEvents();
            start_time = current_time;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
