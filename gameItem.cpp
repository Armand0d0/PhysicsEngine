#include "gameItem.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <vector>


void gameItem::countOBJ(const std::string& filename, int& vertexCount, int& indiceCount) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        vertexCount = indiceCount = 0;
        return;
    }

    vertexCount = 0;
    indiceCount = 0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            vertexCount++;
        } else if (line.substr(0, 2) == "f ") {
            indiceCount+=3;
        }
    }

    file.close();
}

void gameItem::loadMeshFromObjFile(    const std::string& filename, 
        glm::vec3 vertices[],float pointCoords[], int vertexCount,  
        unsigned int indices[], int indiceCount, std::vector<glm::ivec2>& edges){

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
        vertexCount = 0, indiceCount = 0;
        return;
    }
    int **edgesMarker = new int*[vertexCount];
    for(int i=0;i<vertexCount;i++){
        edgesMarker[i] = new int[vertexCount];
        for(int j = 0;j<vertexCount;j++){
            edgesMarker[i][j] = 0;
        }
    }
   
    int vertexCounter= 0, coordCounter= 0, indiceCounter = 0,edgesCounter = 0;
    std::string line;


    while (std::getline(file, line)) {
        std::istringstream iss(line);

        if (line.substr(0, 2) == "v ") {
            // Vertex line
            char v;
            float x, y, z;
            iss >> v >> x >> y >> z;

            vertices[vertexCounter++] = glm::vec3(x, y, z);
            pointCoords[coordCounter] = x;
            pointCoords[coordCounter+1] = y;
            pointCoords[coordCounter+2] = z;
            pointCoords[coordCounter+3] = 0.0;
            pointCoords[coordCounter+4] = 0.0;
            coordCounter+=5;

        } else if (line.substr(0, 2) == "f ") {
            // Face line
            char f;
            std::string v1, v2, v3;
            iss >> f >> v1 >> v2 >> v3;

            auto parseIndex = [](const std::string& token) -> int {
                size_t slashPos = token.find('/');
                if (slashPos != std::string::npos) {
                    return std::stoi(token.substr(0, slashPos)) - 1;
                }
                return std::stoi(token) - 1;
            };


            unsigned int i1 = parseIndex(v1);
            unsigned int i2 = parseIndex(v2);
            unsigned int i3 = parseIndex(v3);

            indices[indiceCounter] = i1;
            indices[indiceCounter+1] = i2;
            indices[indiceCounter+2] = i3;
            indiceCounter+=3;
            if(edgesMarker[i1][i2] == 0){
                edgesMarker[i1][i2] = 1;
                edgesMarker[i2][i1] = 1;
                edges.push_back(glm::ivec2(i1,i2));
            }
            if(edgesMarker[i1][i3] == 0){
                edgesMarker[i1][i3] = 1;
                edgesMarker[i3][i1] = 1;
                edges.push_back(glm::ivec2(i1,i3));
            }
            if(edgesMarker[i2][i3] == 0){
                edgesMarker[i3][i2] = 1;
                edgesMarker[i2][i3] = 1;
                edges.push_back(glm::ivec2(i3,i2));
            }
        }
    }
    for(int j = 0;j<vertexCount;j++){
            free(edgesMarker[j]);
    }
    free(edgesMarker);
    file.close();
}
void gameItem::loadMesh(float* vertices, unsigned int vertexCount, unsigned int* indices, unsigned int indexCount) {
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);


    glGenBuffers(1, &this->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(int), indices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
unsigned int gameItem::loadTexture(const char* fileName) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);// or GL_LINEAR

    string fullFileName = "./textures/";
    fullFileName += fileName;
    int texWidth, texHeight, nrChannels;
    unsigned char* data = stbi_load(fullFileName.c_str(), &texWidth, &texHeight, &nrChannels, 0);
    unsigned int sourcePixelFormat = GL_RGB;
    if (nrChannels == 4) {
        sourcePixelFormat = GL_RGBA;
    }
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, sourcePixelFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture : " << fileName << std::endl;
    }
    stbi_image_free(data);
    return texture;
}
gameItem::gameItem(const char* name, float* vertices, unsigned int vertexCount, unsigned int* indices, unsigned int indexCount, const char* textureFileName) :
    name(name),
    indices(indices),
    indexCount(indexCount),
    vertices(vertices),
    vertexCount(vertexCount),
    position(glm::vec3(0)),
    scale(glm::vec3(1.)),
    rotationAxis(Y),
    rotationAngle(0.),
    edgesColor(glm::vec4(1.,0.,1.,1.)) {

    gameItem::loadMesh(vertices, vertexCount, indices, indexCount);
    this->texture = gameItem::loadTexture(textureFileName);
}
