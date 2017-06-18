//
// Created by Marc on 17/06/2017.
//

#ifndef CGRA_PROJECT_A3_WATER_H
#define CGRA_PROJECT_A3_WATER_H

#define W_RES 128
#define W_G 9.807f   // gravity
#define W_DT 0.01f    // delta time per frame
#define W_DAMP 0.3f

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

        // BASE, INITIAL HEIGHT SAMPLING

        vec2 corner = vec2(a.x, a.y);
        unitX = (b.x - a.x)/W_RES;
        vec2 unitXVec = vec2(unitX, 0);
        unitY = (b.y - a.y)/W_RES;
        vec2 unitYVec = vec2(0, unitY);
        for (int x = 0; x < W_RES; x++) {
            for (int y = 0; y < W_RES; y++) {
                base[x][y] = 5-5.0f*((x + y) - W_RES)/W_RES;
                // corner + unitXVec * x + unitYVec * y
                heightPrev[x][y] = height;
                heightCurrent[x][y] = height;
            }
        }


        // TRIDIAGONAL MATRICIES

        float mat[W_RES][W_RES];
        for (int i = 0; i <= 1; i++) {
            sliceView = (SliceView) i;  // alternate rows and columns
            for (unsigned int j = 0; j < W_RES; j++) {
                currentSlice = j;

                mat[0][0] = e0();

                for (int k = 1; k < W_RES-1; k++) {
                    mat[k][k]     = e(k);
                    mat[k - 1][k] = f(k-1);
                    mat[k][k - 1] = mat[k - 1][k];
                }

                mat[W_RES-2][W_RES-1] = f(W_RES-2);
                mat[W_RES-1][W_RES-2] = mat[W_RES-2][W_RES-1];
                mat[W_RES-1][W_RES-1] = eN();

                mats[currentSlice][sliceView] = arma::fmat::fixed<W_RES, W_RES>((const float*) mat);
            }
        }
    }

    void render() {
        // water simulation
        for (int i = 0; i <= 1; i++) {
            sliceView = (SliceView) i;  // alternate rows and columns
            for (unsigned int j = 0; j < W_RES; j++) {
                currentSlice = j;

                // solve linear system Ax = b
                auto hc = this->hVec(current);
                auto hp = this->hVec(prev);
                arma::fvec b = hc + (1 - W_DAMP) * (hc - hp);
                arma::fvec result = arma::solve(matrix(), b);

                if (sliceView == x) {
                    for (int i = 0; i < W_RES; i++) heightCurrent[i][currentSlice] = result(i);
                } else {
                    for (int i = 0; i < W_RES; i++) heightCurrent[currentSlice][i] = result(i);
                }
            }
            // rotate array refs
            auto temp = heightCurrent;
            heightCurrent = heightNext;
            heightNext = heightPrev;
            heightPrev = temp;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gradientTex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, W_RES, W_RES, 0, GL_RG, GL_FLOAT, gradientMap);
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
    // RENDERING
    GLuint vertArray;
    GLuint vertBuffer;

    GLuint shader;

    vec2 boundA;
    vec2 boundB;

    GLuint gradientTex;
    vec2 gradientMap[W_RES][W_RES];

    // SIMULATION
    // using armadillo for large matrix multiplication
    arma::fmat mats[W_RES][2];

    float unitX;
    float unitY;

    float base[W_RES][W_RES];

    float arrays[3][W_RES][W_RES];

    float (*heightCurrent)[W_RES] = arrays[0];
    float (*heightPrev)[W_RES] = arrays[1];
    float (*heightNext)[W_RES] = arrays[2];

    // state variables to simplify method signatures
    unsigned int currentSlice;
    enum SliceView : char {x = 0, z = 1};
    SliceView sliceView = x;

    enum Snapshot : char {next = 1, current = 0, prev = -1};

    arma::fvec hVec(Snapshot time) {
        const float *array;
        switch (time) {
            case next:
                array = (const float *) heightNext;
                break;
            case current:
                array = (const float *) heightCurrent;
                break;
            case prev:
                array = (const float *) heightPrev;
                break;
        }

        if (currentSlice == x) {
            float x[W_RES];
            for (int i = 0; i < W_RES; i++) x[i] = heightCurrent[i][currentSlice];
            return arma::fvec::fixed<W_RES>((const float *) x);
        } else {
            return arma::fvec::fixed<W_RES>((const float *) &array[currentSlice]);
        }
    }

    arma::fmat matrix() {
        return mats[currentSlice][sliceView];
    }

    // depth of water
    float d(int i) {
        return h(i) - b(i);
    }

    // height of water
    float h(int i) {
        if (sliceView == x) {
            return heightCurrent[i][currentSlice];
        } else {
            return heightCurrent[currentSlice][i];
        }
    }

    // base of geography
    float b(int i) {
        if (sliceView == x) {
            return base[i][currentSlice];
        } else {
            return base[currentSlice][i];
        }
    }

    // matrix building functions

    inline float unitLength() {
        if (sliceView == x) {
            return unitX;
        } else {
            return unitY;
        }
    }

    float e0() {
        return 1 + W_G * W_DT * W_DT * (d(0) + d(1)) / (2 * unitLength() * unitLength());
    }

    float eN() {
        return 1 + W_G * W_DT * W_DT * (d(W_RES - 2) + d(W_RES - 1)) / (2 * unitLength() * unitLength());
    }

    float e(int i) {
        return 1 + W_G * W_DT * W_DT * (d(i-1) + 2*d(i) + d(i+1)) / (2 * unitLength() * unitLength());
    }

    float f(int i) {
        return - W_G * W_DT * W_DT * (d(i) + d(i+1)) / (2 * unitLength() * unitLength());
    }
};


#endif //CGRA_PROJECT_A3_WATER_H
