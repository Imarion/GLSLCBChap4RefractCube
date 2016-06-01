#include "refractcube.h"

#include <QtGlobal>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QTime>

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>

#include <cmath>
#include <cstring>

MyWindow::~MyWindow()
{
    if (mProgramSkybox != 0) delete mProgramSkybox;
    if (mProgramObject != 0) delete mProgramObject;
}

MyWindow::MyWindow()
    : mProgramSkybox(0), mProgramObject(0), currentTimeMs(0.0f), currentTimeS(0.0f), tPrev(0.0f), angle(1.5708f), rotSpeed(M_PI / 2.0f)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
    create();

    resize(800, 600);

    mContext = new QOpenGLContext(this);
    mContext->setFormat(format);
    mContext->create();

    mContext->makeCurrent( this );

    mFuncs = mContext->versionFunctions<QOpenGLFunctions_4_3_Core>();
    if ( !mFuncs )
    {
        qWarning( "Could not obtain OpenGL versions object" );
        exit( 1 );
    }
    if (mFuncs->initializeOpenGLFunctions() == GL_FALSE)
    {
        qWarning( "Could not initialize core open GL functions" );
        exit( 1 );
    }

    initializeOpenGLFunctions();

    QTimer *repaintTimer = new QTimer(this);
    connect(repaintTimer, &QTimer::timeout, this, &MyWindow::render);
    repaintTimer->start(1000/60);

    QTimer *elapsedTimer = new QTimer(this);
    connect(elapsedTimer, &QTimer::timeout, this, &MyWindow::modCurTime);
    elapsedTimer->start(1);       
}

void MyWindow::modCurTime()
{
    currentTimeMs++;
    currentTimeS=currentTimeMs/1000.0f;
}

void MyWindow::initialize()
{
    CreateVertexBuffer();
    initShaders();
    initMatrices();

    //PrepareTexture(GL_TEXTURE0, GL_TEXTURE_2D, "../Media/cement.jpg", false);
    //PrepareTexture(GL_TEXTURE1, GL_TEXTURE_2D, "../Media/moss.png", true);
    PrepareCubeMap(GL_TEXTURE0, "../media/cubemapnight/night", true);

    //mRotationMatrixLocation = mProgram->uniformLocation("RotationMatrix");

    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
}

void MyWindow::CreateVertexBuffer()
{
    // *** Teapot
    mFuncs->glGenVertexArrays(1, &mVAOTeapot);
    mFuncs->glBindVertexArray(mVAOTeapot);

    QMatrix4x4 transform;
    //transform.translate(QVector3D(0.0f, 1.5f, 0.25f));
    mTeapot = new Teapot(14, transform);

    // Create and populate the buffer objects
    unsigned int TeapotHandles[4];
    glGenBuffers(4, TeapotHandles);

    glBindBuffer(GL_ARRAY_BUFFER, TeapotHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, (3 * mTeapot->getnVerts()) * sizeof(float), mTeapot->getv(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, TeapotHandles[1]);
    glBufferData(GL_ARRAY_BUFFER, (3 * mTeapot->getnVerts()) * sizeof(float), mTeapot->getn(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, TeapotHandles[2]);
    glBufferData(GL_ARRAY_BUFFER, (2 * mTeapot->getnVerts()) * sizeof(float), mTeapot->gettc(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TeapotHandles[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * mTeapot->getnFaces() * sizeof(unsigned int), mTeapot->getelems(), GL_STATIC_DRAW);

    // Setup the VAO
    // Vertex positions
    mFuncs->glBindVertexBuffer(0, TeapotHandles[0], 0, sizeof(GLfloat) * 3);
    mFuncs->glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(0, 0);

    // Vertex normals
    mFuncs->glBindVertexBuffer(1, TeapotHandles[1], 0, sizeof(GLfloat) * 3);
    mFuncs->glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(1, 1);

    // Vertex texure coordinates
    mFuncs->glBindVertexBuffer(2, TeapotHandles[2], 0, sizeof(GLfloat) * 2);
    mFuncs->glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(2, 2);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TeapotHandles[3]);

    mFuncs->glBindVertexArray(0);

    // *** Skybox
    mFuncs->glGenVertexArrays(1, &mVAOSkybox);
    mFuncs->glBindVertexArray(mVAOSkybox);    

    mSkyBox = new SkyBox();

    // Create and populate the buffer objects
    unsigned int SkyboxHandles[2];
    glGenBuffers(2, SkyboxHandles);

    glBindBuffer(GL_ARRAY_BUFFER, SkyboxHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, (3 * mSkyBox->getnVerts()) * sizeof(float), mSkyBox->getv(), GL_STATIC_DRAW);    

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyboxHandles[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSkyBox->getnFaces() * sizeof(unsigned int), mSkyBox->getel(), GL_STATIC_DRAW);

    // Setup the VAO
    // Vertex positions
    mFuncs->glBindVertexBuffer(0, SkyboxHandles[0], 0, sizeof(GLfloat) * 3);
    mFuncs->glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(0, 0);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyboxHandles[1]);

    mFuncs->glBindVertexArray(0);
}

void MyWindow::initMatrices()
{
    //ModelMatrixPlane.translate(0.0f, -0.45f, 0.0f);

    ModelMatrixTeapot.translate(0.0f, -1.0f, 0.0f);
    ModelMatrixTeapot.rotate(-90.f, 1.0f, 0.0f, 0.0f);

    //ViewMatrix.lookAt(QVector3D(0.0f, 0.0f, 6.0f), QVector3D(0.0f,2.0f,0.0f), QVector3D(0.0f,1.0f,0.0f));
}

void MyWindow::resizeEvent(QResizeEvent *)
{
    mUpdateSize = true;

    ProjectionMatrix.setToIdentity();
    ProjectionMatrix.perspective(50.0f, (float)this->width()/(float)this->height(), 0.3f, 100.0f);
}

void MyWindow::render()
{
    if(!isVisible() || !isExposed())
        return;

    if (!mContext->makeCurrent(this))
        return;

    static bool initialized = false;
    if (!initialized) {
        initialize();
        initialized = true;
    }

    if (mUpdateSize) {
        glViewport(0, 0, size().width(), size().height());
        mUpdateSize = false;
    }

    float deltaT = currentTimeS - tPrev;
    if(tPrev == 0.0f) deltaT = 0.0f;
    tPrev = currentTimeS;
    angle += rotSpeed * deltaT;
    if (angle > TwoPI) angle -= TwoPI;

    static float EvolvingVal = 0;
    EvolvingVal += 0.1f;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ViewMatrix.setToIdentity();
    QVector3D cameraPos = QVector3D( 7.0f * cos(angle), 2.0f, 7.0f * sin(angle));
    ViewMatrix.lookAt(cameraPos, QVector3D(0.0f,0.0f,0.0f), QVector3D(0.0f,1.0f,0.0f));

    //QMatrix4x4 RotationMatrix;
    //RotationMatrix.rotate(EvolvingVal, QVector3D(0.1f, 0.0f, 0.1f));
    //ModelMatrix.rotate(0.3f, QVector3D(0.1f, 0.0f, 0.1f));
    QVector4D worldLight = QVector4D(0.0f, 0.0f, 0.0f, 1.0f);

    // *** Draw Skybox
    mFuncs->glBindVertexArray(mVAOSkybox);

    glEnableVertexAttribArray(0);

    mProgramSkybox->bind();
    {
        mProgramSkybox->setUniformValue("DrawSkyBox", true);

        mProgramSkybox->setUniformValue("MVP", ProjectionMatrix * ViewMatrix);

        glDrawElements(GL_TRIANGLES, mSkyBox->getnFaces(), GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
        //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));

        glDisableVertexAttribArray(0);        
    }
    mProgramSkybox->release();

    // *** Draw teapot
    mFuncs->glBindVertexArray(mVAOTeapot);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);    

    mProgramObject->bind();
    {
        mProgramObject->setUniformValue("DrawSkyBox", false);
        mProgramObject->setUniformValue("MaterialColor", 0.5f, 0.5f, 0.5f, 1.0f);
        mProgramObject->setUniformValue("ReflectFactor", 0.1f);
        mProgramObject->setUniformValue("MatEta", 0.9f);

        mProgramObject->setUniformValue("WorldCameraPosition", cameraPos);

        QMatrix4x4 mv1 = ViewMatrix * ModelMatrixTeapot;        
        mProgramObject->setUniformValue("ModelMatrix", ModelMatrixTeapot);
        mProgramObject->setUniformValue("MVP", ProjectionMatrix * mv1);

        glDrawElements(GL_TRIANGLES, 6 * mTeapot->getnFaces(), GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);        
    }
    mProgramObject->release();

    mContext->swapBuffers(this);
}

void MyWindow::initShaders()
{
    QOpenGLShader vShader(QOpenGLShader::Vertex);
    QOpenGLShader fShader(QOpenGLShader::Fragment);    
    QFile         shaderFile;
    QByteArray    shaderSource;

    //Skybox shader
    shaderFile.setFileName(":/vshader_skybox.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex skybox compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag          compile: " << fShader.compileSourceCode(shaderSource);

    mProgramSkybox = new (QOpenGLShaderProgram);
    mProgramSkybox->addShader(&vShader);
    mProgramSkybox->addShader(&fShader);
    qDebug() << "shader link: " << mProgramSkybox->link();

    //Object shader
    shaderFile.setFileName(":/vshader_object.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex object compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag          compile: " << fShader.compileSourceCode(shaderSource);

    mProgramObject = new (QOpenGLShaderProgram);
    mProgramObject->addShader(&vShader);
    mProgramObject->addShader(&fShader);
    qDebug() << "shader link: " << mProgramObject->link();
}

void MyWindow::PrepareTexture(GLenum TextureUnit, GLenum TextureTarget, const QString& FileName, bool flip)
{
    QImage TexImg;

    if (!TexImg.load(FileName)) qDebug() << "Erreur chargement texture " << FileName;
    if (flip==true) TexImg=TexImg.mirrored();

    glActiveTexture(TextureUnit);
    GLuint TexID;
    glGenTextures(1, &TexID);
    glBindTexture(TextureTarget, TexID);
    mFuncs->glTexStorage2D(TextureTarget, 1, GL_RGBA8, TexImg.width(), TexImg.height());
    mFuncs->glTexSubImage2D(TextureTarget, 0, 0, 0, TexImg.width(), TexImg.height(), GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    //glTexImage2D(TextureTarget, 0, GL_RGB, TexImg.width(), TexImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void MyWindow::PrepareCubeMap(GLenum TextureUnit, const QString& BaseFileName, bool flip)
{
    glActiveTexture(TextureUnit);

    glGenTextures(1, &SkyBoxTexId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBoxTexId);

    const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
    GLuint targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    // Allocate immutable storage for the cube map texture
    mFuncs->glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, 256, 256);

    // Load each cube-map face
    QImage TexImg;
    for(int i=0; i<6; i++) {
        QString TexName = QString(BaseFileName) + "_" + suffixes[i] + ".png";
        if (!TexImg.load(TexName)) qDebug() << "Erreur chargement texture " << TexName;
        if (flip==true) TexImg=TexImg.mirrored();

        mFuncs->glTexSubImage2D(targets[i], 0, 0, 0, TexImg.width(), TexImg.height(), GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void MyWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_P:
            break;
        case Qt::Key_Up:
            break;
        case Qt::Key_Down:
            break;
        case Qt::Key_Left:
            break;
        case Qt::Key_Right:
            break;
        case Qt::Key_Delete:
            break;
        case Qt::Key_PageDown:
            break;
        case Qt::Key_Home:
            break;
        case Qt::Key_Plus:
            break;
        case Qt::Key_Minus:
            break;
        case Qt::Key_Z:
            break;
        case Qt::Key_Q:
            break;
        case Qt::Key_S:
            break;
        case Qt::Key_D:
            break;
        case Qt::Key_A:
            break;
        case Qt::Key_E:
            break;
        default:
            break;
    }
}

void MyWindow::printMatrix(const QMatrix4x4& mat)
{
    const float *locMat = mat.transposed().constData();

    for (int i=0; i<4; i++)
    {
        qDebug() << locMat[i*4] << " " << locMat[i*4+1] << " " << locMat[i*4+2] << " " << locMat[i*4+3];
    }
}
