#include "skybox.h"

SkyBox::SkyBox()
    :nFaces(36), nVerts(24)
{
    float side = 50.0f;
    float side2 = side / 2.0f;
    v = new float[24*3] {
        // Front
       -side2, -side2, side2,
        side2, -side2, side2,
        side2,  side2, side2,
       -side2,  side2, side2,
       // Right
        side2, -side2, side2,
        side2, -side2, -side2,
        side2,  side2, -side2,
        side2,  side2, side2,
       // Back
       -side2, -side2, -side2,
       -side2,  side2, -side2,
        side2,  side2, -side2,
        side2, -side2, -side2,
       // Left
       -side2, -side2, side2,
       -side2,  side2, side2,
       -side2,  side2, -side2,
       -side2, -side2, -side2,
       // Bottom
       -side2, -side2, side2,
       -side2, -side2, -side2,
        side2, -side2, -side2,
        side2, -side2, side2,
       // Top
       -side2,  side2, side2,
        side2,  side2, side2,
        side2,  side2, -side2,
       -side2,  side2, -side2
    };

    el = new unsigned int[36] {
        0,1,2,0,2,3,
        4,5,6,4,6,7,
        8,9,10,8,10,11,
        12,13,14,12,14,15,
        16,17,18,16,18,19,
        20,21,22,20,22,23
    };
/*
    unsigned int handle[2];
    glGenBuffers(2, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), v, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), el, GL_STATIC_DRAW);

    glGenVertexArrays( 1, &vaoHandle );
    glBindVertexArray(vaoHandle);

    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]);

    glBindVertexArray(0);
    */
}
/*
void SkyBox::render() const
{
    glBindVertexArray(vaoHandle);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
}
*/

float *SkyBox::getv()
{
    return v;
}

int SkyBox::getnVerts()
{
    return nVerts;
}

unsigned int *SkyBox::getel()
{
    return el;
}

int SkyBox::getnFaces()
{
    return nFaces;
}
