OpenGL 3D Cube Renderer
An interactive 3D cube renderer built with C++ and OpenGL. The project demonstrates fundamental computer graphics concepts, including 3D transformations, vertex normals, shader programming, and ambient and diffuse lighting.
Features
- Render a 3D cube using OpenGL 3.3 and GLSL shaders.
- Rotate the cube interactively around the X and Y axes.
- Cycle through a predefined palette of cube colors.
- Move the light source along all three axes to explore real-time shading.
- Use per-face normals for accurate flat-faced lighting.
- Automatically update the viewport when the window is resized.
Technologies
- C++17 — application logic
- OpenGL 3.3 — graphics rendering
- GLFW — window creation and keyboard input
- GLAD — OpenGL function loading (source included)
- GLM — matrix transformations and projection
- GLSL — vertex and fragment shaders
Requirements
A C++17 compiler, make, OpenGL support, and installed GLFW and GLM development libraries. The included Makefile supports Linux/WSL and macOS.

On Ubuntu/Debian or WSL, install the dependencies with:
sudo apt update
sudo apt install build-essential libglfw3-dev libglm-dev libgl1-mesa-dev
On macOS with Homebrew:
brew install glfw glm
Build and Run
From the project directory containing main.cpp and Makefile, run:
make
./assignment_1
Alternatively, build and run with one command:
make run
To remove compiled files:
make clean
Keyboard Controls
Key	Action
↑ / ↓	Rotate around the X axis
← / →	Rotate around the Y axis
C	Cycle through cube colors
J / L	Move light left / right
I / K	Move light up / down
U / O	Move light forward / backward along Z
Esc	Close the application


How It Works
The cube uses 24 vertex entries, four per face, rather than only eight shared corners. This allows each face to have its own normal vector for flat shading. Vertex and fragment shaders transform the cube and calculate ambient and diffuse lighting based on the light position and surface normals.
Project Files
- main.cpp — cube geometry, rendering, shaders, and input controls
- glad.c, glad.h, khrplatform.h — bundled OpenGL loader
- Makefile — build and run commands
