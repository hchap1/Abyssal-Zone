#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"
#include <vector>
#include <string>
#include <numeric>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include "CUSTOM/shader.h"
#include <mutex>
#include "CUSTOM/GLGUI.h"
using namespace std;

tuple<float*, int> tilemapDecoder(vector<vector<int>> tilemap, int tileTextureSize, int windowWidth, int windowHeight, float blockSize) {
    int numOfTriangles = 0;

    float xScale = blockSize / windowWidth;
    float yScale = blockSize / windowHeight;
    float offset = (1.0f / tileTextureSize);
    float yOffset;
    float xDist;
    float yDist;
    float midpointX;
    float midpointY;
    float lightX;
    float lightY;
    float distanceX;
    float distanceY;
    float distance;
    float smallestDistance;
    float closestLightX;
    float closestLightY;
    float lightType;

    vector<vector<float>> lightPositions;
    for (int y = 0; y < tilemap.size(); y++) {
        for (int x = 0; x < tilemap[0].size(); x++) {
            if (tilemap[y][x] == 3 || tilemap[y][x] == 7) {
                vector<float> lightPos = { x * xScale + xScale * 0.5f, y * yScale + yScale * 0.5f, static_cast<float>(tilemap[y][x]) };
                lightPositions.push_back(lightPos);
            }
        }
    }
    size_t totalSize = 0;
    for (const auto& row : tilemap) {
        totalSize += row.size() * 48 * sizeof(float);
    }

    float* vertexData = new float[totalSize];
    int index = 0;
    int tileType = 0;

    for (int height = 0; height < tilemap.size(); height++) {
        for (int width = 0; width < tilemap[0].size(); width++) {
            tileType = tilemap[height][width];
            yOffset = offset * tileType;
            if (tileType != 0) {
                numOfTriangles += 2;

                midpointX = width * xScale + xScale * 0.5f;
                midpointY = height * yScale + yScale * 0.5f;
                smallestDistance = -1.0f;
                closestLightX = 100.0f;
                closestLightY = 100.0f;

                float counter = 0;

                for (vector<float> lightPos : lightPositions) {
                    counter++;
                    lightX = lightPos[0];
                    lightY = lightPos[1];
                    distanceX = abs(midpointX - lightX);
                    distanceY = abs(midpointY - lightY);
                    distance = sqrtf(distanceX * distanceX + distanceY * distanceY);
                    if (distance < smallestDistance || smallestDistance == -1.0f) {
                        smallestDistance = distance;
                        closestLightX = lightX;
                        closestLightY = lightY;
                        lightType = lightPos[2];
                    }
                }

                lightX = closestLightX;
                lightY = closestLightY;

                //Bottom left
                vertexData[index++] = width * xScale;
                vertexData[index++] = height * yScale;
                vertexData[index++] = 0;
                vertexData[index++] = yOffset;
                vertexData[index++] = lightX;
                vertexData[index++] = lightY;
                vertexData[index++] = lightType;
                vertexData[index++] = tileType;

                //Top left
                vertexData[index++] = width * xScale;
                vertexData[index++] = height * yScale + yScale;
                vertexData[index++] = 0;
                vertexData[index++] = yOffset - offset;
                vertexData[index++] = lightX;
                vertexData[index++] = lightY;
                vertexData[index++] = lightType;
                vertexData[index++] = tileType;

                //Bottom left
                vertexData[index++] = width * xScale + xScale;
                vertexData[index++] = height * yScale;
                vertexData[index++] = 1;
                vertexData[index++] = yOffset;
                vertexData[index++] = lightX;
                vertexData[index++] = lightY;
                vertexData[index++] = lightType;
                vertexData[index++] = tileType;

                //Top right
                vertexData[index++] = width * xScale + xScale;
                vertexData[index++] = height * yScale + yScale;
                vertexData[index++] = 1;
                vertexData[index++] = yOffset - offset;
                vertexData[index++] = lightX;
                vertexData[index++] = lightY;
                vertexData[index++] = lightType;
                vertexData[index++] = tileType;

                //Top left
                vertexData[index++] = width * xScale;
                vertexData[index++] = height * yScale + yScale;
                vertexData[index++] = 0;
                vertexData[index++] = yOffset - offset;
                vertexData[index++] = lightX;
                vertexData[index++] = lightY;
                vertexData[index++] = lightType;
                vertexData[index++] = tileType;

                //Bottom left
                vertexData[index++] = width * xScale + xScale;
                vertexData[index++] = height * yScale;
                vertexData[index++] = 1;
                vertexData[index++] = yOffset;
                vertexData[index++] = lightX;
                vertexData[index++] = lightY;
                vertexData[index++] = lightType;
                vertexData[index++] = tileType;
            }
        }
    }
    tuple<float*, int> dataReturn(vertexData, numOfTriangles);
    return dataReturn;
}

class RenderLayer {
public:
    RenderLayer();
    RenderLayer(std::vector<int> attributes, std::string shaderName, std::string textureName, bool flipImageOnLoad) {
        //Create & bind buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //Set up vertex attributes
        int offset = 0;
        int count = 0;
        step = accumulate(attributes.begin(), attributes.end(), 0);
        for (int attribSize : attributes) {
            glVertexAttribPointer(count, attribSize, GL_FLOAT, GL_FALSE, step * sizeof(float), (void*)(offset * sizeof(float)));
            glEnableVertexAttribArray(count);
            count++;
            offset += attribSize;
        }

        //Create empty texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        //Set filtering options (pixel art)
        stbi_set_flip_vertically_on_load(flipImageOnLoad);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        //Load the image file
        int width, height, nrChannels;
        string texturePath = "assets/textures/" + textureName + ".png";

        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

        //Bind image data to texture obj
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //Free the image data to avoid memory leaks
        stbi_image_free(data);

        //Create shader obj for this rendering layer
        string vertexPath = "assets/shaders/" + shaderName + "_vertex_shader.glsl";
        string fragmentPath = "assets/shaders/" + shaderName + "_fragment_shader.glsl";
        shader = Shader(vertexPath.c_str(), fragmentPath.c_str());
    }

    void setVertices(float* vertices, int numTriangles, int numFloatsPerTriangle, unsigned int mode) {
        lock_guard<mutex> lock(mtx);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, numTriangles * numFloatsPerTriangle * sizeof(float), vertices, mode);
    }
    void draw(int numTriangles) {
        lock_guard<mutex> lock(mtx);
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.use();
        glDrawArrays(GL_TRIANGLES, 0, numTriangles * 3);
    }
    void setFloat(string name, float value) {
        lock_guard<mutex> lock(mtx);
        shader.use();
        shader.setFloat(name, value);
    }
    void setBool(string name, bool value) {
        lock_guard<mutex> lock(mtx);
        shader.use();
        shader.setBool(name, value);
    }

private:
    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    int step;
    Shader shader;
    mutex mtx;
};

class Renderer {
public:
    Renderer(int width, int height, std::string windowName) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);
        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        glfwSetCursorPosCallback(window, this->mouseCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        lastFrame = glfwGetTime();
    }
    void updateDisplay() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    void fillScreen(int r, int g, int b) {
        glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    bool isRunning() {
        return !glfwWindowShouldClose(window);
    }
    float getBlockWidth() { return 100.0f / width; }
    float getBlockHeight() { return 100.0f / height; }
    bool getKeyDown(unsigned int keycode) { return glfwGetKey(window, keycode); }
    float getDeltaTime() {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        return deltaTime;
    }
    float mouseX;
    float mouseY;
    void setMousePosition(GLFWwindow* window, double x, double y) {
        mouseX = x;
        mouseY = y;
    }
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        // Wrapper
        Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
        if (renderer) {
            renderer->setMousePosition(window, xpos, ypos);
        }
    }

private:
    GLFWwindow* window;
    int width, height;
    float lastFrame;
};