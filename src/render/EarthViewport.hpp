#pragma once
#include "astronomy/SolarGeometry.hpp"
#include "astronomy/SpiceEphemeris.hpp"
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QString>
#include <QVector3D>
namespace atlas {
class EarthViewport final:public QOpenGLWidget,protected QOpenGLFunctions{
 Q_OBJECT
public:
 explicit EarthViewport(QWidget* parent=nullptr);
 ~EarthViewport() override;
 void setSolarGeometry(const astro::SolarGeometry& geometry);
 void setMoonBodyFrame(const astro::LunarBodyFrame& frame);
 void clearMoonBodyFrame();
 bool preciseMoonFrameActive()const{return preciseMoonFrame_;}
 void focusSubEarth();
 bool setTextureFile(const QString& path);
 enum class Scene { Earth, Moon };
 void setScene(Scene nextScene);
 Scene scene() const { return scene_; }
 bool setMoonTextureFile(const QString& path);
 bool hasObservedTexture() const { return scene_==Scene::Earth ? texture_!=0 : moonTexture_!=0; }
 void resetCamera();
 void focusDaylight();
protected:
 void initializeGL() override;
 void resizeGL(int width,int height) override;
 void paintGL() override;
 void mousePressEvent(QMouseEvent* event) override;
 void mouseMoveEvent(QMouseEvent* event) override;
 void wheelEvent(QWheelEvent* event) override;
private:
 struct Vertex{float px,py,pz,nx,ny,nz,u,v;};
 void buildSphere();
 bool reloadTexture();
 bool reloadMoonTexture();
 bool uploadImageTexture(const QString& path, GLuint& target);
 void cleanupGL();
 QOpenGLShaderProgram shader_;
 QOpenGLBuffer vertices_{QOpenGLBuffer::VertexBuffer},indices_{QOpenGLBuffer::IndexBuffer};
 QOpenGLVertexArrayObject vao_;
 int indexCount_=0;
 GLuint texture_=0;
 GLuint moonTexture_=0;
 QString moonTexturePath_;
 Scene scene_=Scene::Earth;
 bool glInitialized_=false;
 QString texturePath_;
 astro::SolarGeometry solar_{};
 QVector3D sunWorld_{1.0f,0.0f,0.0f};
 QVector3D moonSunMeScene_{1.0f,0.0f,0.0f};
 QVector3D moonEarthMeScene_{1.0f,0.0f,0.0f};
 bool preciseMoonFrame_=false;
 bool followSubEarth_=true;
 float yaw_=39.0f,pitch_=19.0f,radius_=3.3f;
 QPoint lastMouse_;
};
}
