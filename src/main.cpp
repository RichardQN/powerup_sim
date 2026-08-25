// src/main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void errorCallback(int code, const char* description) {
    std::cerr << "GLFW Error [" << code << "]: " << description << "\n";
}

int main() {
    glfwSetErrorCallback(errorCallback);   // add this before glfwInit()

    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);  // force X11, don't rely on env vars

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    // 1. Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // 2. Tell GLFW what GL version/profile you want (match what you picked in GLAD)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Create the window (this also creates the GL context, but doesn't make it current yet)
    GLFWwindow* window = glfwCreateWindow(800, 600, "My Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // 4. Make the context current on this thread
    glfwMakeContextCurrent(window);

    // 5. NOW load GLAD — must happen after a context exists and is current
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // (optional) confirm it worked
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

    // 6. Set the viewport to match window size
    glViewport(0, 0, 800, 600);

    // 7. Main loop
    while (!glfwWindowShouldClose(window)) {
        // input handling would go here

        // clear the screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // rendering goes here (nothing to draw yet)

        glfwSwapBuffers(window);   // present the frame
        glfwPollEvents();          // handle window/input events
    }

    // 8. Cleanup
    glfwTerminate();
    return 0;
}