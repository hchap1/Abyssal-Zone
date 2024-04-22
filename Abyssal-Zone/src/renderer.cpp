#define STB_IMAGE_IMPLEMENTATION
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <STB/stb_image.h>
#include <CUSTOM/shader.h>
#include <tuple>
#include <vector>
#include <numeric>
#include <CUSTOM/tilemap.h>
#include <CUSTOM/renderer.h>
#include <filesystem>
namespace fs = std::filesystem;
using namespace std;

tuple<float*, int> tilemapDecoder(vector<vector<int>> tilemap, int tileTextureSize, int windowWidth, int windowHeight, vector<vector<int>> lightLocations) {
    int numOfTriangles = 0;

    float xScale = 300.0f / windowWidth;
    float yScale = 300.0f / windowHeight;
    float offset = (1.0f / tileTextureSize);
    float yOffset;

    size_t totalSize = 0;
    for (const auto& row : tilemap) {
        totalSize += row.size() * 30 * sizeof(float);
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
                //Bottom left
                vertexData[index++] = width * xScale;
                vertexData[index++] = height * yScale;
                vertexData[index++] = 0;
                vertexData[index++] = yOffset;
                vertexData[index++] = tileType;

                //Top left
                vertexData[index++] = width * xScale;
                vertexData[index++] = height * yScale + yScale;
                vertexData[index++] = 0;
                vertexData[index++] = yOffset - offset;
                vertexData[index++] = tileType;

                //Bottom left
                vertexData[index++] = width * xScale + xScale;
                vertexData[index++] = height * yScale;
                vertexData[index++] = 1;
                vertexData[index++] = yOffset;
                vertexData[index++] = tileType;

                //Top right
                vertexData[index++] = width * xScale + xScale;
                vertexData[index++] = height * yScale + yScale;
                vertexData[index++] = 1;
                vertexData[index++] = yOffset - offset;
                vertexData[index++] = tileType;

                //Top left
                vertexData[index++] = width * xScale;
                vertexData[index++] = height * yScale + yScale;
                vertexData[index++] = 0;
                vertexData[index++] = yOffset - offset;
                vertexData[index++] = tileType;

                //Bottom left
                vertexData[index++] = width * xScale + xScale;
                vertexData[index++] = height * yScale;
                vertexData[index++] = 1;
                vertexData[index++] = yOffset;
                vertexData[index++] = tileType;
            }
        }
    }
    cout << "NUM TRIANGLES: " << numOfTriangles << " WIDTH " << tilemap.size() << " HEIGHT " << tilemap[0].size() << endl;
    tuple<float*, int> dataReturn(vertexData, numOfTriangles);
    return dataReturn;
}

RenderLayer::RenderLayer() {}
RenderLayer::RenderLayer(std::vector<int> attributes, std::string shaderName, std::string textureName, bool flipImageOnLoad) {
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
    

    bool doesExist = fs::exists(texturePath);
    if (doesExist) {
        cout << "FOUND " << texturePath << endl;
    }
    else {
        cout << "COULD NOT FIND " << texturePath << endl;
    }

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

void RenderLayer::setVertices(float* vertices, int numTriangles, int numFloatsPerTriangle, unsigned int mode) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, numTriangles * numFloatsPerTriangle * sizeof(float), vertices, mode);
}

void RenderLayer::draw(int numTriangles) {
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.use();
    glDrawArrays(GL_TRIANGLES, 0, numTriangles * 3);
}

void RenderLayer::setFloat(string name, float value) {
    shader.use();
    shader.setFloat(name, value);
}

void RenderLayer::setBool(string name, bool value) {
    shader.use();
    shader.setBool(name, value);
}

Renderer::Renderer(int width, int height, std::string windowName) : width(width), height(height) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    lastFrame = glfwGetTime();
}

void Renderer::updateDisplay() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::fillScreen(int r, int g, int b) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Renderer::isRunning() {
    return !glfwWindowShouldClose(window);
}

float Renderer::getBlockWidth() {
    return 100.0f / width;
}

float Renderer::getBlockHeight() {
    return 100.0f / height;
}

bool Renderer::getKeyDown(unsigned int keycode) {
    return glfwGetKey(window, keycode);
}

float Renderer::getDeltaTime() {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    return deltaTime;
}