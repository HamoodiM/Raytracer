#include <glad/glad.h>
#include <GLFW/glfw3.h>

// This is a placeholder - in a real project you would generate this with glad
// For now, we'll just include the necessary OpenGL functions

GLADloadproc gladLoadGLLoader(void* loader) {
    // Placeholder implementation
    return (GLADloadproc)glfwGetProcAddress;
}
