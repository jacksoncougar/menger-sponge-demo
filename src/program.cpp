#define MOUSE_SENSITIVITY 200
#define GLEW_STATIC
#define MENGER_DEPTH 2
#define MAX_MENGER_DEPTH 7

/**
 * @file grahics-assignment-1-part1.cpp
 * @author Jackson Wiebe jacksonwiebe1@ucalgary.ca
 * @version 0.1
 * @date 2019-10-19
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "math.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

void key_callback(GLFWwindow *window,
                  int key,
                  int scancode,
                  int action,
                  int mods);

void toggle_shader();

void capture_mouse(GLFWwindow *window);

void release_mouse(GLFWwindow *window);
void update_instances(GLuint instancebuffer, unsigned int iterations);

void window_size_callback(GLFWwindow *window, int width, int height);

void mouse_button_callback(GLFWwindow *window,
                           int button, int action, int mods);

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

void update_projection_matrix();
void update_view_matrix();

struct
{
    unsigned int width = 900;
    unsigned int height = 600;
} g_viewport;

struct Camera
{
    float yaw = 0.0;
    float pitch = 0.0;
    float zoom = 0.5;
    enum Projection
    {
        PERSPECTIVE,
        ORTHO
    } mode = PERSPECTIVE;
    bool captured = true;
} g_camera;

struct
{
    GLuint currentProgram;
    GLuint program1;
    GLuint program2;
    GLuint instancebuffer;
    GLuint vao;
    GLuint arraybuffer;
    Cube cube;
    int iterations = MENGER_DEPTH;
    int instancecount = 1;
    int elementCount = 0;
} g_gl;

void menger(const int iteration)
{
    static std::vector<Vector4f> locations = {
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
    if (iteration < 1)
    {
        glBegin(GL_QUADS);
        for (auto vertex : g_gl.cube.vertices)
        {
            glColor3f(vertex.color.x, vertex.color.y, vertex.color.z);
            glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
        }
        glEnd();
        return;
    }

    glPushMatrix();
    glScaled(1. / 3, 1. / 3, 1. / 3);
    for (auto location : locations)
    {
        glPushMatrix();
        glTranslated(location.x, location.y, location.z);
        menger(iteration - 1);
        glPopMatrix();
    }
    glPopMatrix();
}

int main()
{
    GLFWwindow *window;

    // init glfw

    if (!glfwInit())
    {
        return -1;
    }
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(g_viewport.width,
                              g_viewport.height,
                              "Menger",
                              nullptr,
                              nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    {
        auto version = (const unsigned char *)glGetString(GL_VERSION);
        std::cout << version << std::endl;
    }

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        g_camera.captured = true;
    }

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSwapInterval(1);

    // init glew

    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        return -1;
    }

    if (GLEW_VERSION_2_1)
    {
        std::cout << "2.1 Supported" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    update_view_matrix();
    update_projection_matrix();
    while (!glfwWindowShouldClose(window))
    {
        update_view_matrix();

        glClearColor(.1, .1, .1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        menger(g_gl.iterations);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void cursor_position_callback(GLFWwindow *window,
                              double x,
                              double y)
{
    static double mx(x);
    static double my(y);

    if (!g_camera.captured)
    {
        mx = x;
        my = y;
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        g_camera.zoom += (my - y) / MOUSE_SENSITIVITY;
        if (g_camera.zoom < 0.2)
            g_camera.zoom = 0.2;
    }
    else
    {
        g_camera.pitch += (mx - x) / MOUSE_SENSITIVITY;
        g_camera.yaw += (my - y) / MOUSE_SENSITIVITY;
    }

    mx = x;
    my = y;

    update_view_matrix();
    update_projection_matrix();
}

void update_view_matrix()
{
    auto scale = g_camera.mode == Camera::Projection::PERSPECTIVE ? g_camera.zoom : 1;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mat4x4 S(scale, 0, 0, 0,
             0, scale, 0, 0,
             0, 0, scale, 0,
             0, 0, 0, 1);

    mat4x4 R1(cos(g_camera.pitch), 0, sin(g_camera.pitch), 0,
              0, 1, 0, 0,
              -sin(g_camera.pitch), 0, cos(g_camera.pitch), 0,
              0, 0, 0, 1);

    mat4x4 R2(1, 0, 0, 0,
              0, cos(g_camera.yaw), -sin(g_camera.yaw), 0,
              0, sin(g_camera.yaw), cos(g_camera.yaw), 0,
              0, 0, 0, 1);

    mat4x4 T(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, -2, 1);

    auto result = S * R1 * R2 * T;

    glLoadTransposeMatrixf(&result.m00);
}

void update_projection_matrix()
{
    float fov = 30.0;
    float aspect = static_cast<float>(g_viewport.width) / g_viewport.height;
    float near = 0.3;
    float top = tanf(fov * M_PI / 360.0) * near;
    float far = 50.0;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;
    glViewport(0, 0, g_viewport.width, g_viewport.height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (g_camera.mode == Camera::Projection::PERSPECTIVE)
    {
        mat4x4 projection(
            2 * near / (right - left), 0, (right + left) / (right - left), 0,
            0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
            0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
            0, 0, -1, 0);

        glLoadMatrixf(&projection.m00);
    }
    else
    {
        auto right = 0.5 * aspect / g_camera.zoom;
        auto top = 0.5 / g_camera.zoom;
        mat4x4 projection(
            1. / right, 0, 0, 0,
            0, 1. / top, 0, 0,
            0, 0, -2. / (far - near), -((far + near) / (far - near)),
            0, 0, 0, 1.);
        glLoadMatrixf(&projection.m00);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        capture_mouse(window);
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
    glfwGetFramebufferSize(window, &width, &height);
    g_viewport.width = width;
    g_viewport.height = height;
    update_view_matrix();
    update_projection_matrix();
}

void key_callback(GLFWwindow *window,
                  int key,
                  int scancode,
                  int action,
                  int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        release_mouse(window);

    if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
    {
        g_camera.mode = g_camera.mode == Camera::Projection::PERSPECTIVE ? Camera::Projection::ORTHO : Camera::Projection::PERSPECTIVE;
        update_view_matrix();
        update_projection_matrix();
    }
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
        g_gl.iterations > 0 ? --g_gl.iterations : 0;
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
        g_gl.iterations < MAX_MENGER_DEPTH ? ++g_gl.iterations : MAX_MENGER_DEPTH;
}

void release_mouse(GLFWwindow *window)
{
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPos(window, g_viewport.width / 2., g_viewport.height / 2.);
    g_camera.captured = false;
}

void capture_mouse(GLFWwindow *window)
{
    g_camera.captured = true;
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
