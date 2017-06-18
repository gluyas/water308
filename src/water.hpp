//
// Created by Marc on 17/06/2017.
//

#ifndef CGRA_PROJECT_A3_WATER_H
#define CGRA_PROJECT_A3_WATER_H

#define W_RES 128
#define W_G 9.807f   // gravity
#define W_DT 0.1f    // delta time per frame

#include "opengl.hpp"
#include "cgra_math.hpp"
#include "simple_shader.hpp"
#include <armadillo>

using namespace std;
using namespace cgra;

class Water {
public:
    Water(vec2 a, vec2 b, float height) {

        vec3 quad[4];
        if (a.x > b.x) {
            float temp = a.x;
            a.x = b.x;
            b.x = temp;
        }
        if (a.y > b.y) {
            float temp = a.y;
            a.y = b.y;
            b.y = temp;
        }
        quad[0] = vec3(a.x, height, a.y);
        quad[1] = vec3(b.x, height, a.y);
        quad[2] = vec3(b.x, height, b.y);
        quad[3] = vec3(a.x, height, b.y);


        boundA = a;
        boundB = b;

        // STORE GEOMETRY

        glGenVertexArrays(1, &vertArray);
        glBindVertexArray(vertArray);

        glGenBuffers(1, &vertBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 4, quad, GL_STATIC_READ);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // SHADER

        shader = makeShaderProgramFromFile(
                {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
                { "./res/shaders/water.vert", "./res/shaders/water.frag"}
        );

        // GRADIENT MAP

        for (int i = 0; i < W_RES; i++) {
            for (int j = 0; j < W_RES; j++) {
                //gradientMap[i][j] = (i + j)%2 == 0? vec2(0,0) : vec2(1,1);
                //gradientMap[i][j] = j > 50 ? vec2(0,0) : vec2(1,1);
                //gradientMap[i][j] = vec2((float) i/W_RES, (float) j/W_RES);
                //gradientMap[i][j] = vec2(cos((float) i/W_RES * 12), sin((float) j/W_RES * 12));
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &gradientTex); // Generate texture ID
        glBindTexture(GL_TEXTURE_2D, gradientTex); // Bind it as a 2D texture

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Finnly, actually fill the data into our texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, W_RES, W_RES, 0, GL_RG, GL_FLOAT, gradientMap);
        //gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RG32F, W_RES, W_RES, 0, GL_UNSIGNED_BYTE, gradientMap);

        // TRIDIAGONAL MATRIX

        float mat[W_RES][W_RES];
        for (int i = 1; i < W_RES - 1; i++) {
            mat[i][i] = e(i);
            mat[i-1][i] = f(i);
            mat[i][i-1] = f(i);
        }
    }

    void render() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gradientTex);
        glUniform1i(glGetUniformLocation(GL_TEXTURE0, "waterGradientMap"), 0);

        glUniform2f(glGetUniformLocation(shader, "waterBoundA"), boundA.x, boundA.y);
        glUniform2f(glGetUniformLocation(shader, "waterBoundB"), boundB.x, boundB.y);

        glBindVertexArray(vertArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        glUseProgram(0);
        glDisable(GL_BLEND);
    }

private:
    GLuint vertArray;
    GLuint vertBuffer;

    GLuint shader;

    vec2 boundA;
    vec2 boundB;

    GLuint gradientTex;
    vec2 gradientMap[W_RES][W_RES];

    arma::sp_fmat tridiag{W_RES, W_RES};
    float dX;
    bool 

    // depth function
    float d(int i) {

    }

    // matrix building functions

    float e0() {
        return 1 + W_G * W_DT * W_DT * (d(0) + d(1)) / (2 * dX * dX);
    }

    float eN() {
        return 1 + W_G * W_DT * W_DT * (d(W_RES - 2) + d(W_RES - 1)) / (2 * dX * dX);
    }

    float e(int i) {
        return 1 + W_G * W_DT * W_DT * (d(i-1) + 2*d(i) + d(i+1)) / (2 * dX * dX);
    }

    float f(int i) {
        return - W_G * W_DT * W_DT * (d(i) + d(i+1)) / (2 * dX * dX);
    }


};


#endif //CGRA_PROJECT_A3_WATER_H
