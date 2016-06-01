#ifndef SKYBOX_H
#define SKYBOX_H


class SkyBox
{
private:
    int nFaces;

    // Vertices
    float *v;
    int nVerts;

    // Elements
    unsigned int *el;

public:
    SkyBox();

    float *getv();
    int    getnVerts();
    unsigned int *getel();

    int    getnFaces();
};


#endif // SKYBOX_H
