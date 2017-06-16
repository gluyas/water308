//
// Created by Marc on 17/06/2017.
//

#ifndef CGRA_PROJECT_A3_WATER_H
#define CGRA_PROJECT_A3_WATER_H

#include "opengl.hpp"
#include "cgra_math.hpp"
#include "simple_shader.hpp"

using namespace std;
using namespace cgra;

class Water {
public:
    Water(vec3 verts[], size_t len, GLenum drawType) {
        this->numVerts = len;
        this->drawType = drawType;

        glGenVertexArrays(1, &vertArray);
        glBindVertexArray(vertArray);

        glGenBuffers(1, &vertBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * len, verts, GL_STATIC_READ);

        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        shader = makeShaderProgramFromFile(
                {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
                { "./res/shaders/water.vert", "./res/shaders/water.frag"}
        );
    }

    void render() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(shader);
        glBindVertexArray(vertArray);
        glDrawArrays(drawType, 0, numVerts);
        glBindVertexArray(0);
        glUseProgram(0);
        glDisable(GL_BLEND);
    }

private:
    GLuint vertArray;
    GLuint vertBuffer;

    GLuint shader;

    size_t numVerts;
    GLenum drawType;
};


#endif //CGRA_PROJECT_A3_WATER_H
