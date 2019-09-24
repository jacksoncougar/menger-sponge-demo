#define GLEW_STATIC
#define MENGER_DEPTH 3
/**
 * @file grpahics-assignment-1-part1.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2019-09-19
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "math.h"
#include <cmath>
#include <iostream>
#include <algorithm>

void print_error(int, const char *c);
void key_handler(GLFWwindow *window, int key, int scancode, int action, int mods);

void menger(const Cube &cube, const int iteration);

void window_size_callback(GLFWwindow *window, int width, int height);

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

float rotation = 0.0;

int main()
{
    Cube test;

    GLFWwindow *window;

    if (!glfwInit())
    {
        return -1;
    }

    glfwSetErrorCallback(print_error);

    glfwWindowHint(GLFW_DEPTH_BITS, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    const unsigned int width = 900;
    const unsigned int height = 900;

    window = glfwCreateWindow(width, height, "Menger", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_handler);

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    auto version = (const unsigned char *)glGetString(GL_VERSION);
    std::cout << version << std::endl;

    glfwSwapInterval(1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float fov = 70.0;
    float aspect = width / height;
    float near = .30;
    float far = 500.0;
    float top = tanf(fov * M_PI / 360.0) * near;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;
    float angle = 0.0;

    glFrustum(left, right, bottom, top, near, far);
    GLuint indexbuffer;
    GLuint va;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glVertexPointer(4, GL_FLOAT, 32, &test.vertices[0].position);
    glColorPointer(4, GL_FLOAT, 32, &test.vertices[0].color);

    glPolygonMode(GL_FRONT, GL_POINT);
    glPolygonMode(GL_BACK, GL_POINT);

    while (!glfwWindowShouldClose(window))
    {
        // todo draw

        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(0, 0, -1.5);
        glRotated(rotation, 0, 1, 0);
        angle += 0.5;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        menger(test, MENGER_DEPTH);
        glPopMatrix();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

const std::vector<Vector4f> locations = {
    {-1, -1, -1, 1},
    {-1, 0, -1, 1},
    {-1, 1, -1, 1},
    {0, -1, -1, 1},
    {0, 1, -1, 1},
    {1, -1, -1, 1},
    {1, 0, -1, 1},
    {1, 1, -1, 1},
    {-1, -1, 0, 1},
    {-1, 1, 0, 1},
    {1, -1, 0, 1},
    {1, 1, 0, 1},
    {-1, -1, 1, 1},
    {-1, 0, 1, 1},
    {-1, 1, 1, 1},
    {0, -1, 1, 1},
    {0, 1, 1, 1},
    {1, -1, 1, 1},
    {1, 0, 1, 1},
    {1, 1, 1, 1},

};
void menger(const Cube &cube, const int iteration)
{
    if (iteration < 1)
    {
        glDrawArrays(GL_QUADS, 0, cube.vertices.size());
        return;
    }

    glPushMatrix();
    glScaled(1. / 3, 1. / 3, 1. / 3);
    for (auto location : locations)
    {
        glPushMatrix();
        glTranslated(location.x, location.y, location.z);
        menger(cube, iteration - 1);
        glPopMatrix();
    }
    glPopMatrix();
}

void print_error(int, const char *c)
{
    std::cout << c;
}

static void cursor_position_callback(GLFWwindow *window,
                                     double xpos,
                                     double ypos)
{
    int amount = static_cast<int>(xpos) % 360;
    rotation = amount;
    std::cout << rotation << std::endl;
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float fov = 70.0;
    float aspect = width / static_cast<float>(height);
    float near = .30;
    float far = 500.0;
    float top = tanf(fov * M_PI / 360.0) * near;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;
    float angle = 0.0;

    glFrustum(left, right, bottom, top, near, far);
}

void key_handler(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    return;
}
