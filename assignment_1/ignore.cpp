// =============================================================================
// CPSC 484 - Assignment 1 - OpenGL Mesh Viewer (STARTER)
// =============================================================================
// This file is NOT a fill-in-the-blanks starter. It gives you the parts that
// are the same in literally every OpenGL program -- opening a window and
// getting a context (Step 1), loading OpenGL's function pointers (Step 2),
// and the compile/link utility functions for turning GLSL text into a usable
// shader program -- because none of that is specific to THIS assignment, and
// you already built it once, understanding every line, in the Assignment 0
// demo (cpsc484_a1_demo_letterviewer.cpp / its line-by-line commented
// twin). Re-typing it here wouldn't teach you anything new.
//
// Everything below a "// TODO (2.x)" comment is yours to write from scratch.
// There are no predefined variables named cubeColor or lightPos waiting for
// you to fill in a value -- you decide what state your program needs and how
// to store it, the same way you'll have to on every assignment after this
// one. The section numbers in the TODOs match the Assignment 1 Instructions
// document; that's where the actual requirements and grading weights live.
// This file only tells you WHERE things go, not WHAT to write.
//
// Before you write a single line here, make sure you can explain -- to
// yourself, out loud -- every piece of the Assignment 0 demo: why GLFW hints
// have to be set before glfwCreateWindow(), what a VAO records versus what a
// VBO holds, why glVertexAttribPointer's stride/offset have to match your
// vertex layout exactly, and why compileShader() checks GL_COMPILE_STATUS.
// If any of those feel shaky, that demo (not this file) is where to go back
// to. This assignment assumes you already own that material.
// =============================================================================

#include "glad.h"          // OpenGL function loader -- must be included before glfw3.h
#include <GLFW/glfw3.h>    // window/context creation, input, timing
#include <cmath>
#include <cstdlib>         // std::getenv -- used by isRunningUnderWSL() below
#include <fstream>         // std::ifstream -- used by isRunningUnderWSL() below
#include <iostream>        // std::cerr / std::cout for error and debug messages
#include <string>          // std::string -- used by isRunningUnderWSL() below, and by the titleString you'll add next

// TODO (2.1): declare your own window-title string here. See Assignment 1
// Instructions, Section 2.1. Something like:
//     std::string titleString = "Fall 2026 - Assignment 1 - <Your Full Name>";
std::string titleString = "Fall 2026 - Assignment 1 - Bardia Shariat";
// -----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
// C++ requires a function to be declared before it's used. main() (further
// down) calls these, so they're declared here and defined later in the file.
// -----------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // called by GLFW whenever the window is resized
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // called by GLFW on every key press/release/repeat
unsigned int compileShader(unsigned int type, const char* source);         // compiles one GLSL shader, returns its OpenGL ID
unsigned int createShaderProgram(const char* vertexSrc, const char* fragmentSrc); // compiles + links both shaders into one usable program

// Screen dimensions -- passed to glfwCreateWindow() below. Feel free to
// change these, but keep them as named constants rather than magic numbers
// scattered through the file.
const unsigned int SCR_WIDTH = 800;  // window width in pixels
const unsigned int SCR_HEIGHT = 600; // window height in pixels

// TODO (2.3): declare your vertex shader and fragment shader source here, as
// C++ raw string literals (see the Assignment 0 demo for the R"GLSL(...)GLSL"
// pattern and why it's safer than a plain R"(...)"). At minimum your vertex
// shader needs:
//   - a position input attribute and a normal input attribute
//   - a "transform" uniform (mat4) to place/rotate the mesh
// and your fragment shader needs:
//   - a "color" uniform (vec3) for the current cube color
//   - some simple ambient + diffuse shading using the light's position/
//     direction, so the cube's faces are visibly shaded differently rather
//     than being flat silhouettes (see the demo's fragment shader for one
//     way to do this -- yours doesn't have to match it exactly).
//
const char *vertexShaderSource = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;

uniform mat4 transform;

void main() {
  Normal = mat3(transform) * aNormal;
  gl_Position = transform * vec4(aPos,1.0);
}
)GLSL";

const char *fragmentShaderSource = R"GLSL(
#version 330 core
out vec4 FragColor;
in vec3 Normal;
uniform vec3 color;
uniform vec3 lightDirection;

void main() {
  vec3 N = normalize(Normal);
  vec3 lightDir = normalize(lightDirection);

  float ambient = 0.5;
  float diffuse = max(dot(N, lightDir), 0.0) * 1.2;

  FragColor = vec4(color * (ambient + diffuse), 1.0);
}
)GLSL";

// TODO (2.2): declare whatever state your mesh needs. At minimum you'll want
// somewhere to put your vertex data (positions + normals) and your index
// data once you've decided on a layout -- see Section 2.2 for the required
// float vertices[] / unsigned int indices[] shape. You'll also need VAO/VBO/
// EBO ids once you get to uploading that data to the GPU.
// Each vertex: x, y, z, normalX, normalY, normalZ
float vertices[] = {
    // Front (+Z)
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

    // Back (-Z)
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

    // Left (-X)
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,

    // Right (+X)
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

    // Top (+Y)
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

    // Bottom (-Y)
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f};

// Two triangles per face.
unsigned int indices[] = {
    0, 1, 2, 2, 3, 0,       // Front
    4, 5, 6, 6, 7, 4,       // Back
    8, 9, 10, 10, 11, 8,    // Left
    12, 13, 14, 14, 15, 12, // Right
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
};

unsigned int VAO, VBO, EBO;

constexpr float colorPalette[][3] = {
    {0.8f, 0.4f, 0.2f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f}
};
constexpr unsigned int colorCount = sizeof(colorPalette) / sizeof(colorPalette[0]);
unsigned int currentColorIndex = 0;
float cubeColor[] = {colorPalette[0][0], colorPalette[0][1], colorPalette[0][2]};
float lightDirection[] = {0.4f, 0.6f, 1.0f};
float rotationAngles[] = {0.4f, 0.6f, 0.0f}; // X/Y/Z angles in radians.

// -----------------------------------------------------------------------------
// PLATFORM DETECTION (Linux/WSL only -- a no-op on Windows/macOS)
// -----------------------------------------------------------------------------
// Carried over from the Assignment 0 demo unchanged: under WSLg, GLFW's
// default Wayland backend has a known window-resize bug, so on WSL
// specifically we ask GLFW to use X11 instead. See the demo's own comments
// (right above its isRunningUnderWSL()) for the full explanation -- there's
// nothing assignment-specific to change here.
bool isRunningUnderWSL() {
    if (std::getenv("WSL_DISTRO_NAME") != nullptr) return true;
    if (std::getenv("WSL_INTEROP") != nullptr) return true;

    std::ifstream versionFile("/proc/version");
    if (versionFile) {
        std::string contents((std::istreambuf_iterator<char>(versionFile)),
                              std::istreambuf_iterator<char>());
        for (char& c : contents) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (contents.find("microsoft") != std::string::npos) return true;
    }
    return false;
}

/* *************************************************** */

int main() {
    // ---- Step 0: steer GLFW away from WSLg's buggy Wayland backend -----
#if defined(GLFW_VERSION_MAJOR) && (GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4))
    if (isRunningUnderWSL() && glfwPlatformSupported(GLFW_PLATFORM_X11)) {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    }
#endif

    // ---- Step 1: create a window + OpenGL context via GLFW -------------
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Ask for an OpenGL 3.3 Core Profile context, same as Assignment 0.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // required on macOS to get a core-profile context at all; harmless no-op on Windows/Linux

    // TODO (2.1): pass your titleString.c_str() as the window title below
    // instead of the placeholder "Assignment 1" literal.
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, titleString.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Register our callbacks. GLFW calls these automatically -- we never
    // call them ourselves.
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // ---- Step 2: load OpenGL function pointers via GLAD ----------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    const GLint transformLocation = glGetUniformLocation(shaderProgram, "transform");
    const GLint colorLocation = glGetUniformLocation(shaderProgram, "color");
    const GLint lightLocation = glGetUniformLocation(shaderProgram, "lightDirection");

    glEnable(GL_DEPTH_TEST); // near surfaces should hide far ones -- you'll want this once you have a 3D cube

    // TODO (2.2): build your cube's vertex/index data and upload it to the
    // GPU here (glGenVertexArrays / glGenBuffers / glBindBuffer /
    // glBufferData / glVertexAttribPointer / glEnableVertexAttribArray),
    // once you've declared the arrays and layout above. This happens once,
    // before the render loop -- not every frame.
    // Create the vertex array and buffers.

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW);

    // Upload index data.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(indices),
        indices,
        GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void *)0);
    glEnableVertexAttribArray(0);

    // Normal attribute:
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    // ---- Step 6 (numbering matches the Assignment 0 demo): render loop --
    while (!glfwWindowShouldClose(window)) {
        // TODO: poll any continuously-held keys here, if you're using that
        // input style for anything (see the demo's processInput() for the
        // pattern, and its INPUT HANDLING comment block for when polling is
        // the right tool vs. when the key_callback below is).

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        const float cx = std::cos(rotationAngles[0]);
        const float sx = std::sin(rotationAngles[0]);
        const float cy = std::cos(rotationAngles[1]);
        const float sy = std::sin(rotationAngles[1]);
        const float cz = std::cos(rotationAngles[2]);
        const float sz = std::sin(rotationAngles[2]);
        // Column-major rotation matrix: Rz * Ry * Rx.
        const float transform[] = {
            cz * cy, sz * cy, -sy, 0.0f,
            cz * sy * sx - sz * cx, sz * sy * sx + cz * cx, cy * sx, 0.0f,
            cz * sy * cx + sz * sx, sz * sy * cx - cz * sx, cy * cx, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, transform);
        glUniform3fv(colorLocation, 1, cubeColor);
        glUniform3fv(lightLocation, 1, lightDirection);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(sizeof(indices) / sizeof(indices[0])),
                       GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Cleanup ---------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

// Called every time the window is resized.
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

// Called by GLFW whenever a key is pressed, released, or repeated.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// -----------------------------------------------------------------------------
// SHADER COMPILE HELPERS
// -----------------------------------------------------------------------------
// Carried over from the Assignment 0 demo, unchanged -- this is reusable
// boilerplate, not something specific to this assignment's mesh. You WILL
// need to call createShaderProgram() with your own shader source strings
// (Section 2.3) -- that's the assignment-specific part.
//
// ALWAYS check compile/link status like this. When you get a blank screen
// later, this is almost always where the answer is -- read the console
// output before you touch anything else.

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}
