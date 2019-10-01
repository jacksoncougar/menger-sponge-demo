#define MOUSE_SENSITIVITY 200
#define GLEW_STATIC
#define MENGER_DEPTH 3
#define MAX_MENGER_DEPTH 7
#define LEGACY true

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

void key_callback(GLFWwindow *window, int key,
                  int scancode, int action, int mods);

void toggle_shader();

void capture_mouse(GLFWwindow *window);

void release_mouse(GLFWwindow *window);

void menger(mat4x4 root,
            std::vector<mat4x4> &instances,
            const int iteration);

void update_instances(GLuint instancebuffer, unsigned int iterations);

void window_size_callback(GLFWwindow *window, int width, int height);

void mouse_button_callback(GLFWwindow *window,
                           int button, int action, int mods);

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

void create_vao();

GLuint create_shader_program(std::string vertexShaderFilename,
                             std::string fragmentShaderFilename);

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
    int iterations = MENGER_DEPTH;
    int instancecount = 1;
    int elementCount = 0;
} g_gl;

void draw_modern_gl(GLFWwindow *window)
{
    glBindVertexArray(g_gl.vao);
    glDrawArraysInstanced(GL_QUADS,
                          0,
                          g_gl.elementCount,
                          g_gl.instancecount);
}
void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam)
{
    std::cout << message << std::endl;
}

std::vector<Vector4f> locations = {
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

void initialize_modern_gl()
{
    glDebugMessageCallback(MessageCallback, nullptr);
    {
        auto version = (const unsigned char *)glGetString(GL_VERSION);
        std::cout << version << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glPolygonMode(GL_FRONT, GL_FLAT);
    glPolygonMode(GL_BACK, GL_FLAT);

    g_gl.program1 = create_shader_program("shaders/vertex-shader.glsl",
                                          "shaders/fragment-shader.glsl");
    g_gl.program2 = create_shader_program("shaders/vertex-shader-flat.glsl",
                                          "shaders/fragment-shader-flat.glsl");
    toggle_shader();
    create_vao();
    update_view_matrix();
    update_projection_matrix();
}

void initialize_legacy_gl()
{

    static Cube cube;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float fov = 70.0;
    float aspect = g_viewport.width / g_viewport.height;
    float near = .30;
    float far = 500.0;
    float top = tanf(fov * M_PI / 360.0) * near;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;
    float angle = 0.0;

    glFrustum(left, right, bottom, top, near, far);

    glVertexPointer(4, GL_FLOAT, sizeof(Vertex), &cube.vertices[0].position);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &cube.vertices[0].color);
}

void menger(const int iteration)
{
    if (iteration < 1)
    {
        glDrawArrays(GL_QUADS, 0, g_gl.elementCount);
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

void draw_legacy_gl(GLFWwindow *window)
{
    glBegin(GL_QUADS);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0, 0, -1.5);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menger(g_gl.iterations);
    glPopMatrix();
    glEnd();
}

int main()
{
    GLFWwindow *window;

    // init glfw

    if (!glfwInit())
    {
        return -1;
    }

    glfwSetErrorCallback(error_callback);

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

    // if (glewInit() != GLEW_OK)
    // {
    //     glfwTerminate();
    //     return -1;
    // }

    if (LEGACY)
    {
        initialize_legacy_gl();
    }
    else
    {
        initialize_modern_gl();
    }

    // start render loop

    float time = 0;
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (LEGACY)
        {
            draw_legacy_gl(window);
        }
        else
        {
            draw_modern_gl(window);
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

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
    g_gl.instancecount = instances.size();
}

void error_callback(int, const char *c)
{
    std::cout << c;
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

    R2.multiply(T);
    R1.multiply(R2);
    S.multiply(R1);

    {
        auto view_uniform_location =
            glGetUniformLocation(g_gl.program1, "view");

        glUniformMatrix4fv(view_uniform_location, 1, false, &S.m00);
    }
    {
        auto view_uniform_location =
            glGetUniformLocation(g_gl.program2, "view");

        glUniformMatrix4fv(view_uniform_location, 1, false, &S.m00);
    }
}

void update_projection_matrix()
{
    float fov = 30.0;
    float aspect = static_cast<float>(g_viewport.width) / g_viewport.height;
    float near = 0.01;
    float top = tanf(fov * M_PI / 360.0) * near;
    float far = 50.0;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;

    if (g_camera.mode == Camera::Projection::PERSPECTIVE)
    {
        mat4x4 projection(
            2 * near / (right - left), 0, (right + left) / (right - left), 0,
            0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
            0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
            0, 0, -1, 0);

        {
            auto projection_uniform_location =
                glGetUniformLocation(g_gl.currentProgram, "projection");

            glUniformMatrix4fv(projection_uniform_location, 1, true, &projection.m00);
        }
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
        {
            auto projection_uniform_location =
                glGetUniformLocation(g_gl.currentProgram, "projection");

            glUniformMatrix4fv(projection_uniform_location, 1, true, &projection.m00);
        }
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
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window,
                  int key,
                  int scancode,
                  int action,
                  int mods)
{
    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
        toggle_shader();

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        release_mouse(window);

    if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
    {
        g_camera.mode = g_camera.mode == Camera::Projection::PERSPECTIVE ? Camera::Projection::ORTHO : Camera::Projection::PERSPECTIVE;
        update_view_matrix();
        update_projection_matrix();
    }
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
        update_instances(g_gl.instancebuffer,
                         g_gl.iterations > 0 ? --g_gl.iterations : 0);
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
        update_instances(g_gl.instancebuffer,
                         g_gl.iterations < MAX_MENGER_DEPTH ? ++g_gl.iterations : MAX_MENGER_DEPTH);
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

void toggle_shader()
{
    if (g_gl.currentProgram == g_gl.program1)
    {
        glUseProgram(g_gl.program2);
        g_gl.currentProgram = g_gl.program2;
    }
    else
    {
        glUseProgram(g_gl.program1);
        g_gl.currentProgram = g_gl.program1;
    }
    update_view_matrix();
    update_projection_matrix();
}

void create_vao()
{
    Cube cube;
    std::vector<mat4x4> instances = {};
    instances.reserve(pow(20, MENGER_DEPTH));

    menger(mat4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
           instances,
           MENGER_DEPTH);
    g_gl.instancecount = instances.size();

    glGenVertexArrays(1, &g_gl.vao);
    glBindVertexArray(g_gl.vao);

    glGenBuffers(1, &g_gl.arraybuffer);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl.arraybuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Vertex) * cube.vertices.size(),
                 &cube.vertices[0],
                 GL_STATIC_DRAW);
    g_gl.elementCount = cube.vertices.size();

    glGenBuffers(1, &g_gl.instancebuffer);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl.instancebuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4x4) * instances.size(),
                 &instances[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, g_gl.arraybuffer);

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

    glBindBuffer(GL_ARRAY_BUFFER, g_gl.instancebuffer);
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

    glBindVertexArray(0);
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