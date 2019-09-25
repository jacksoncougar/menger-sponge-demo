#define GLEW_STATIC
#define MENGER_DEPTH 4
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

void menger(mat4x4 root,
            std::vector<mat4x4> &instances,
            const int iteration);

void menger(const Cube &cube, const int iteration);

void window_size_callback(GLFWwindow *window, int width, int height);

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

GLuint loadShaders();

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
    float near = 1;
    float far = 500.0;
    float top = tanf(fov * M_PI / 360.0) * near;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;
    float angle = 0.0;

    mat4x4 view(1, 0, 0, 0,
                0, cos(rotation), -sin(rotation), 0,
                0, sin(rotation), cos(rotation), 0,
                0, 0, -3, 1);
    mat4x4 projection(
        2 * near / (right - left), 0, (right + left) / (right - left), 0,
        0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
        0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
        0, 0, -1, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // glVertexPointer(4, GL_FLOAT, 32, &test.vertices[0].position);
    // glColorPointer(4, GL_FLOAT, 32, &test.vertices[0].color);

    auto program = loadShaders();

    GLuint arraybuffer;
    GLuint instancebuffer;
    GLuint vao;

    std::vector<mat4x4> instances = {};
    instances.reserve(pow(20, MENGER_DEPTH));

    menger(mat4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
           instances,
           MENGER_DEPTH);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &test.vertexbufferid);
    glBindBuffer(GL_ARRAY_BUFFER, test.vertexbufferid);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Vertex) * test.vertices.size(),
                 &test.vertices[0],
                 GL_STATIC_DRAW);

    glGenBuffers(1, &instancebuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instancebuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4x4) * instances.size(),
                 &instances[0], GL_STATIC_DRAW);

    glUseProgram(program);
    auto projection_uniform_location =
        glGetUniformLocation(program, "projection");
    auto view_uniform_location =
        glGetUniformLocation(program, "view");

    glUniformMatrix4fv(projection_uniform_location, 1, false, &projection.m00);
    glUniformMatrix4fv(view_uniform_location, 1, false, &view.m00);

    glBindBuffer(GL_ARRAY_BUFFER, test.vertexbufferid);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          (void *)0);

    glVertexAttribPointer(1,
                          4,
                          GL_FLOAT,
                          GL_TRUE,
                          sizeof(Vertex),
                          (void *)sizeof(Vector4f));

    glBindBuffer(GL_ARRAY_BUFFER, instancebuffer);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(mat4x4),
                          (void *)0);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_TRUE, sizeof(mat4x4),
                          (void *)(1 * sizeof(Vector4f)));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_TRUE, sizeof(mat4x4),
                          (void *)(2 * sizeof(Vector4f)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_TRUE, sizeof(mat4x4),
                          (void *)(3 * sizeof(Vector4f)));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

    glPolygonMode(GL_FRONT, GL_FLAT);
    glPolygonMode(GL_BACK, GL_FLAT);

    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        // todo draw

        mat4x4 view(1, 0, 0, 0,
                    0, cos(rotation), -sin(rotation), 0,
                    0, sin(rotation), cos(rotation), 0,
                    0, 0, -1, 1);
        glUniformMatrix4fv(view_uniform_location, 1, false, &view.m00);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_QUADS, 0,
                              test.vertices.size(), instances.size());

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

void menger(mat4x4 root,
            std::vector<mat4x4> &instances,
            const int iteration)
{
    if (iteration < 1)
    {
        instances.emplace_back(root);
        return;
    }

    for (auto location : locations)
    {
        mat4x4 O = root;
        mat4x4 S(1. / 3, 0, 0, 0,
                 0, 1. / 3, 0, 0,
                 0, 0, 1. / 3, 0,
                 0, 0, 0, 1);
        mat4x4 T(1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 location.x, location.y, location.z, 1);

        T.multiply(S);
        O.multiply(T);

        menger(O, instances, iteration - 1);
    }
}
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
    rotation = amount * 0.02;
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

GLuint loadShaders()
{
    typedef GLuint ProgramID;
    typedef GLuint ShaderID;

    ShaderID vertex = glCreateShader(GL_VERTEX_SHADER);
    ShaderID fragment = glCreateShader(GL_FRAGMENT_SHADER);

    {
        std::string source = ""
                             "#version 330 core\n"
                             "layout(location = 0) in vec4 position;"
                             "layout(location = 1) in vec4 color;"
                             "layout(location = 2) in mat4x4 instanceWorld;"
                             "uniform mat4x4 projection;"
                             "uniform mat4x4 view;"
                             "out vec4 vcolor;"
                             "void main() {"
                             "  gl_Position.xyzw = projection * view * instanceWorld * position;"
                             "  vcolor = color;"
                             "}";

        const char *sourcePointer = source.c_str();
        glShaderSource(vertex, 1, &sourcePointer, nullptr);
        glCompileShader(vertex);

        GLint Result = GL_FALSE;
        int InfoLogLength;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(vertex,
                               InfoLogLength,
                               NULL,
                               &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }
    }
    {
        std::string source = ""
                             "#version 330 core\n"
                             "in vec4 vcolor;"
                             "out vec4 color;"
                             "void main() {"
                             "  color = vcolor;"
                             "}";

        const char *sourcePointer = source.c_str();
        glShaderSource(fragment, 1, &sourcePointer, nullptr);
        glCompileShader(fragment);

        GLint Result = GL_FALSE;
        int InfoLogLength;
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(fragment,
                               InfoLogLength,
                               NULL,
                               &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }
    }
    ProgramID program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);

    glDetachShader(program, vertex);
    glDetachShader(program, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}