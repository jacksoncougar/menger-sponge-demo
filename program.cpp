#define MOUSE_SENSITIVITY 200
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
#include <fstream>
#include <sstream>

void error_callback(int, const char *c);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void toggle_shader();

void menger(mat4x4 root,
            std::vector<mat4x4> &instances,
            const int iteration);

void update_instances(GLuint instancebuffer, unsigned int iterations);

void window_size_callback(GLFWwindow *window, int width, int height);

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

GLuint create_shader_program(std::string vertexShaderFilename,
                             std::string fragmentShaderFilename);

int iterations = MENGER_DEPTH;
int instancecount = 1;
float rotation = 0.0;
float rotation2 = 0.0;
float zoom = 0;
float mx = 0 / 0, my = 0 / 0; // Nan
GLuint currentprogram(0);
GLuint program1;
GLuint program2;

GLuint instancebuffer;

int main()
{
    Cube test;
    GLFWwindow *window;

    if (!glfwInit())
    {
        return -1;
    }

    glfwSetErrorCallback(error_callback);

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

    glfwSetKeyCallback(window, key_callback);

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    {
        auto version = (const unsigned char *)glGetString(GL_VERSION);
        std::cout << version << std::endl;
    }

    glfwSwapInterval(1);

    float fov = 30.0;
    float aspect = width / height;
    float near = 1;
    float top = tanf(fov * M_PI / 360.0) * near;
    float far = 500.0;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;
    float angle = 0.0;

    mat4x4 view(1, 0, 0, 0,
                0, cos(rotation), -sin(rotation), 0,
                0, sin(rotation), cos(rotation), 0,
                0, 0, -2 + zoom, 1);
    mat4x4 projection(
        2 * near / (right - left), 0, (right + left) / (right - left), 0,
        0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
        0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
        0, 0, -1, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    program1 = create_shader_program("shaders/vertex-shader.glsl",
                                     "shaders/fragment-shader.glsl");
    program2 = create_shader_program("shaders/vertex-shader-flat.glsl",
                                     "shaders/fragment-shader-flat.glsl");
    toggle_shader();

    GLuint arraybuffer;
    GLuint vao;

    std::vector<mat4x4> instances = {};
    instances.reserve(pow(20, MENGER_DEPTH));

    menger(mat4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
           instances,
           MENGER_DEPTH);
    instancecount = instances.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &arraybuffer);
    glBindBuffer(GL_ARRAY_BUFFER, arraybuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Vertex) * test.vertices.size(),
                 &test.vertices[0],
                 GL_STATIC_DRAW);

    glGenBuffers(1, &instancebuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instancebuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4x4) * instances.size(),
                 &instances[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arraybuffer);

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
        // update scene rotation
        mat4x4 R1(cos(rotation), 0, sin(rotation), 0,
                  0, 1, 0, 0,
                  -sin(rotation), 0, cos(rotation), 0,
                  0, 0, 0, 1);

        mat4x4 R2(1, 0, 0, 0,
                  0, cos(rotation2), -sin(rotation2), 0,
                  0, sin(rotation2), cos(rotation2), 0,
                  0, 0, -2 + zoom, 1);
        R1.multiply(R2);

        auto projection_uniform_location =
            glGetUniformLocation(currentprogram, "projection");
        auto view_uniform_location =
            glGetUniformLocation(currentprogram, "view");

        glUniformMatrix4fv(projection_uniform_location, 1, false, &projection.m00);
        glUniformMatrix4fv(view_uniform_location, 1, false, &R1.m00);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_QUADS,
                              0,
                              test.vertices.size(),
                              instancecount);

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

void update_instances(GLuint instancebuffer, unsigned int iterations)
{
    std::vector<mat4x4> instances = {};
    instances.reserve(pow(20, iterations));

    menger(mat4x4(1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1),
           instances,
           iterations);

    glBindBuffer(GL_ARRAY_BUFFER, instancebuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(mat4x4) * instances.size(),
                 &instances[0],
                 GL_DYNAMIC_DRAW);
    instancecount = instances.size();
}

void error_callback(int, const char *c)
{
    std::cout << c;
}

void cursor_position_callback(GLFWwindow *window,
                              double x,
                              double y)
{
    if (isnan(mx) || isnan(my))
    {
        mx = x;
        my = y;
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        zoom += (my - y) / MOUSE_SENSITIVITY;
    }
    else
    {
        rotation += (mx - x) / MOUSE_SENSITIVITY;
        rotation2 += (my - y) / MOUSE_SENSITIVITY;
    }

    mx = x;
    my = y;
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
    // todo update the View and Projection matrices.
}

void key_callback(GLFWwindow *window,
                  int key,
                  int scancode,
                  int action,
                  int mods)
{
    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
        toggle_shader();
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
        update_instances(instancebuffer,
                         iterations > 0 ? --iterations : 0);
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
        update_instances(instancebuffer,
                         iterations < 5 ? ++iterations : 5);
}

void toggle_shader()
{
    if (currentprogram == program1)
    {
        glUseProgram(program2);
        currentprogram = program2;
    }
    else
    {
        glUseProgram(program1);
        currentprogram = program1;
    }
}
GLuint create_shader_program(std::string vertexShaderFilename,
                             std::string fragmentShaderFilename)
{
    typedef GLuint ProgramID;
    typedef GLuint ShaderID;

    ShaderID vertex = glCreateShader(GL_VERTEX_SHADER);
    ShaderID fragment = glCreateShader(GL_FRAGMENT_SHADER);

    std::ifstream file(vertexShaderFilename);
    if (!file.is_open())
    {
        throw;
    }
    std::string vertexShaderSource(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    file.close();
    file.open(fragmentShaderFilename, std::ios_base::in);
    if (!file.is_open())
    {
        throw;
    }
    std::string fragmentShaderSource(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    {
        const char *sourcePointer = vertexShaderSource.c_str();
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
        const char *sourcePointer = fragmentShaderSource.c_str();
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