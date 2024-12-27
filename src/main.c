#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "util/types.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200
#define PADDLE_SPEED 10
#define BALL_SPEED 5
#define BALL_SIZE 20
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 100
#define TICK 5 // milliseconds

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
    ball->dx = BALL_SPEED;
    ball->dy = BALL_SPEED;
}

void drawRectangle(f32 x, f32 y, f32 width, f32 height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
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
    trajectory->y = ballCopy.y;

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
        calculateTrajectory(trajectory, ball, leftPaddle, rightPaddle);
    }

    // right paddle collision
    if (ball->x + BALL_SIZE >= rightPaddle->x && ball->y + BALL_SIZE >= rightPaddle->y && ball->y <= rightPaddle->y + PADDLE_HEIGHT && ball->dx > 0) {
        if (ball->y + BALL_SIZE > rightPaddle->y && ball->y < rightPaddle->y + PADDLE_HEIGHT) {
            ball->dx *= -1;
            ball->x = rightPaddle->x - BALL_SIZE;
        }
        calculateTrajectory(trajectory, ball, leftPaddle, rightPaddle);
    }
    
    // horizontal borders
    if (ball->x <= 0 || ball->x + BALL_SIZE >= SCREEN_WIDTH) {
        ball->x = SCREEN_WIDTH / 2;
        ball->y = SCREEN_HEIGHT / 2;
        ball->dx *= -1;
        calculateTrajectory(trajectory, ball, leftPaddle, rightPaddle);
    }
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
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
            updateBall(&ball, &leftPaddle, &rightPaddle, &trajectory);
            updatePaddle(&leftPaddle, &rightPaddle, &trajectory);
            drawRectangle(leftPaddle.x, leftPaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT);
            drawRectangle(rightPaddle.x, rightPaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT);
            drawRectangle(ball.x, ball.y, BALL_SIZE, BALL_SIZE);
            glfwSwapBuffers(window);
            glfwPollEvents();
            start_time = current_time;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
