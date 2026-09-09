#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "scene_math.h"

#include <algorithm>
#include <iostream>
#include <vector>

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength > 0 ? logLength : 1, '\0');
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        std::cerr << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                  << " shader compilation failed:\n" << log.data() << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void errorCallback(int errorCode, const char* description)
{
    std::cerr << "GLFW error [" << errorCode << "]: "
              << description << '\n';
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

struct OrbitCamera {
    // Angles are radians; distance and positions are in world units.
    float yaw = 0.65f;
    float pitch = 0.45f;
    float distance = 6.0f;

    Vec3 eye() const
    {
        return {distance * std::cos(pitch) * std::sin(yaw),
                0.5f + distance * std::sin(pitch),
                distance * std::cos(pitch) * std::cos(yaw)};
    }
};

void processInput(GLFWwindow* window, OrbitCamera& camera, float elapsedSeconds)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    auto held = [window](int key) { return glfwGetKey(window, key) == GLFW_PRESS; };
    // Orbit at 1.2 radians/second and zoom at 3 world units/second.
    camera.yaw += (held(GLFW_KEY_RIGHT) - held(GLFW_KEY_LEFT)) * 1.2f * elapsedSeconds;
    camera.pitch += (held(GLFW_KEY_UP) - held(GLFW_KEY_DOWN)) * 1.2f * elapsedSeconds;
    bool zoomIn = held(GLFW_KEY_EQUAL) || held(GLFW_KEY_KP_ADD);
    bool zoomOut = held(GLFW_KEY_MINUS) || held(GLFW_KEY_KP_SUBTRACT);
    camera.distance += (zoomOut - zoomIn) * 3.0f * elapsedSeconds;
    camera.yaw = std::remainder(camera.yaw, 6.283185307f);
    camera.pitch = std::clamp(camera.pitch, 0.1f, 1.45f);
    camera.distance = std::clamp(camera.distance, 2.5f, 15.0f);
    if (held(GLFW_KEY_R)) camera = OrbitCamera{};
}

int main()
{
    glfwSetErrorCallback(errorCallback);

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    // Start with a widely supported version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(800, 600, "Powerup Simulator", nullptr, nullptr);

    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (gladLoadGLLoader(
            reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(
        window,
        &framebufferWidth,
        &framebufferHeight
    );
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // Enable vertical synchronization.
    glfwSwapInterval(1);

    std::cout << "OpenGL vendor: "
              << reinterpret_cast<const char*>(glGetString(GL_VENDOR))
              << '\n'
              << "OpenGL renderer: "
              << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
              << '\n'
              << "OpenGL version: "
              << reinterpret_cast<const char*>(glGetString(GL_VERSION))
              << std::endl;

    const char* vertexSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
out vec3 vertexColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Apply object -> world -> camera -> clip transforms, right to left.
    gl_Position = projection * view * model * vec4(position, 1.0);
    vertexColor = color;
}
)glsl";

    const char* fragmentSource = R"glsl(
#version 330 core
in vec3 vertexColor;
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor, 1.0);
}
)glsl";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    // The linked program retains its executable after the shaders are deleted.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength > 0 ? logLength : 1, '\0');
        glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        std::cerr << "Shader program linking failed:\n" << log.data() << '\n';
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // One unit cube (36 vertices), then an 8x8 ground plane (6 vertices).
    // Positions are object-space world units; each vertex still has XYZ + RGB.
    const float vertices[] = {
        // Front
        -0.50f, -0.50f, 0.50f, 0.20f, 0.65f, 1.00f,
        0.50f, -0.50f, 0.50f, 0.20f, 0.65f, 1.00f,
        0.50f, 0.50f, 0.50f, 0.20f, 0.65f, 1.00f,
        -0.50f, -0.50f, 0.50f, 0.20f, 0.65f, 1.00f,
        0.50f, 0.50f, 0.50f, 0.20f, 0.65f, 1.00f,
        -0.50f, 0.50f, 0.50f, 0.20f, 0.65f, 1.00f,
        // Back
        0.50f, -0.50f, -0.50f, 0.15f, 0.40f, 0.70f,
        -0.50f, -0.50f, -0.50f, 0.15f, 0.40f, 0.70f,
        -0.50f, 0.50f, -0.50f, 0.15f, 0.40f, 0.70f,
        0.50f, -0.50f, -0.50f, 0.15f, 0.40f, 0.70f,
        -0.50f, 0.50f, -0.50f, 0.15f, 0.40f, 0.70f,
        0.50f, 0.50f, -0.50f, 0.15f, 0.40f, 0.70f,
        // Right
        0.50f, -0.50f, 0.50f, 0.15f, 0.50f, 0.85f,
        0.50f, -0.50f, -0.50f, 0.15f, 0.50f, 0.85f,
        0.50f, 0.50f, -0.50f, 0.15f, 0.50f, 0.85f,
        0.50f, -0.50f, 0.50f, 0.15f, 0.50f, 0.85f,
        0.50f, 0.50f, -0.50f, 0.15f, 0.50f, 0.85f,
        0.50f, 0.50f, 0.50f, 0.15f, 0.50f, 0.85f,
        // Left
        -0.50f, -0.50f, -0.50f, 0.10f, 0.35f, 0.60f,
        -0.50f, -0.50f, 0.50f, 0.10f, 0.35f, 0.60f,
        -0.50f, 0.50f, 0.50f, 0.10f, 0.35f, 0.60f,
        -0.50f, -0.50f, -0.50f, 0.10f, 0.35f, 0.60f,
        -0.50f, 0.50f, 0.50f, 0.10f, 0.35f, 0.60f,
        -0.50f, 0.50f, -0.50f, 0.10f, 0.35f, 0.60f,
        // Top
        -0.50f, 0.50f, 0.50f, 0.45f, 0.80f, 1.00f,
        0.50f, 0.50f, 0.50f, 0.45f, 0.80f, 1.00f,
        0.50f, 0.50f, -0.50f, 0.45f, 0.80f, 1.00f,
        -0.50f, 0.50f, 0.50f, 0.45f, 0.80f, 1.00f,
        0.50f, 0.50f, -0.50f, 0.45f, 0.80f, 1.00f,
        -0.50f, 0.50f, -0.50f, 0.45f, 0.80f, 1.00f,
        // Bottom
        -0.50f, -0.50f, -0.50f, 0.10f, 0.25f, 0.45f,
        0.50f, -0.50f, -0.50f, 0.10f, 0.25f, 0.45f,
        0.50f, -0.50f, 0.50f, 0.10f, 0.25f, 0.45f,
        -0.50f, -0.50f, -0.50f, 0.10f, 0.25f, 0.45f,
        0.50f, -0.50f, 0.50f, 0.10f, 0.25f, 0.45f,
        -0.50f, -0.50f, 0.50f, 0.10f, 0.25f, 0.45f,
        // Ground plane
        -4.00f, 0.00f, -4.00f, 0.23f, 0.27f, 0.32f,
        -4.00f, 0.00f, 4.00f, 0.23f, 0.27f, 0.32f,
        4.00f, 0.00f, 4.00f, 0.23f, 0.27f, 0.32f,
        -4.00f, 0.00f, -4.00f, 0.23f, 0.27f, 0.32f,
        4.00f, 0.00f, 4.00f, 0.23f, 0.27f, 0.32f,
        4.00f, 0.00f, -4.00f, 0.23f, 0.27f, 0.32f,
    };

    GLuint vertexArray = 0;
    GLuint vertexBuffer = 0;
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // The VAO records the buffer and layout. Stride and offsets are in bytes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          reinterpret_cast<const void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const GLint modelLocation = glGetUniformLocation(program, "model");
    const GLint viewLocation = glGetUniformLocation(program, "view");
    const GLint projectionLocation = glGetUniformLocation(program, "projection");
    glEnable(GL_DEPTH_TEST);
    OrbitCamera camera;
    double previousTime = glfwGetTime();
    std::cout << "Arrow keys: orbit | +/-: zoom | R: reset camera | Escape: quit\n";

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        double currentTime = glfwGetTime();
        // Discard time beyond 100 ms after a stall to avoid a large camera jump.
        float elapsedSeconds = static_cast<float>(std::clamp(currentTime - previousTime, 0.0, 0.1));
        previousTime = currentTime;
        processInput(window, camera, elapsedSeconds);
        if (glfwWindowShouldClose(window)) break;

        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0) {
            glfwWaitEventsTimeout(0.05);
            continue;
        }
        Mat4 view = lookAt(camera.eye(), {0, 0.5f, 0});
        Mat4 projection = perspective(0.785398163f,
            static_cast<float>(framebufferWidth) / framebufferHeight, 0.1f, 50.0f);

        glClearColor(0.05f, 0.02f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vertexArray);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view.values);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection.values);

        // Lift the cube's center so its bottom rests on the ground at y = 0.
        Mat4 cubeModel = translation({0, 0.5f, 0});
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, cubeModel.values);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw ground second: depth testing keeps it from covering the cube.
        Mat4 groundModel = translation({0, 0, 0});
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, groundModel.values);
        glDrawArrays(GL_TRIANGLES, 36, 6);

        glfwSwapBuffers(window);
    }

    // GPU resources belong to this context, so release them before the window.
    glBindVertexArray(0);
    glUseProgram(0);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
