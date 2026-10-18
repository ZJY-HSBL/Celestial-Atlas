#include "render/EarthViewport.hpp"
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QOpenGLContext>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace atlas {
namespace {constexpr double toRadians=3.1415926535897932384626433832795/180.0;}
EarthViewport::EarthViewport(QWidget* parent):QOpenGLWidget(parent){
 setMinimumSize(400,340);
 setMouseTracking(true);
 setFocusPolicy(Qt::StrongFocus);
}
EarthViewport::~EarthViewport(){
 if(context()){makeCurrent();cleanupGL();doneCurrent();}
}
void EarthViewport::cleanupGL(){
 if(texture_){glDeleteTextures(1,&texture_);texture_=0;}
 if(moonTexture_){glDeleteTextures(1,&moonTexture_);moonTexture_=0;}
 vao_.destroy();vertices_.destroy();indices_.destroy();glInitialized_=false;
}
void EarthViewport::setSolarGeometry(const astro::SolarGeometry& geometry){
 solar_=geometry;
 const auto sun=astro::celestialUnitVector(geometry.rightAscensionDeg,geometry.declinationDeg);
 sunWorld_=QVector3D(float(sun.x),float(sun.y),float(sun.z));
 update();
}
bool EarthViewport::setTextureFile(const QString& path){
 QImage check(path);
 if(check.isNull()||check.width()!=2048||check.height()!=1024)return false;
 texturePath_=path;
 if(glInitialized_){makeCurrent();bool ok=reloadTexture();doneCurrent();update();return ok;}
 return true;
}
void EarthViewport::setScene(Scene scene){
 scene_=scene;
 if(scene_==Scene::Moon){
  // Moon survey globe: camera faces mean near side, fixed body orientation.
  // Studio lighting is intentionally NOT an observed lunar phase or libration.
  yaw_=0.0f;pitch_=12.0f;radius_=3.65f;
 }else{resetCamera();}
 update();
}
bool EarthViewport::setMoonTextureFile(const QString& path){
 QImage check(path);
 if(check.isNull()||check.size()!=QSize(2048,1024))return false;
 moonTexturePath_=path;
 if(glInitialized_){makeCurrent();bool ok=reloadMoonTexture();doneCurrent();update();return ok;}
 return true;
}
void EarthViewport::focusDaylight(){
 if(scene_==Scene::Moon){yaw_=0.0f;pitch_=12.0f;radius_=3.65f;update();return;}
 const float pi=3.14159265358979323846f;
 yaw_=std::atan2(sunWorld_.z(),sunWorld_.x())*180.0f/pi;
 pitch_=std::asin(std::clamp(sunWorld_.y(),-1.0f,1.0f))*180.0f/pi;
 radius_=3.85f;
 update();
}
void EarthViewport::resetCamera(){yaw_=39.0f;pitch_=19.0f;radius_=3.3f;update();}
void EarthViewport::initializeGL(){
 initializeOpenGLFunctions();
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
 glClearColor(0.012f,0.024f,0.045f,1.0f);
 if(!shader_.addShaderFromSourceFile(QOpenGLShader::Vertex,":/shaders/earth.vert")
   ||!shader_.addShaderFromSourceFile(QOpenGLShader::Fragment,":/shaders/earth.frag")
   ||!shader_.link()){
  qWarning("Unable to compile Earth shaders: %s",qPrintable(shader_.log()));
  return;
 }
 buildSphere();
 glInitialized_=true;
 if(!texturePath_.isEmpty())reloadTexture();
 if(!moonTexturePath_.isEmpty())reloadMoonTexture();
}
void EarthViewport::buildSphere(){
 constexpr int lonSteps=144,latSteps=72;
 std::vector<Vertex> vertices;
 std::vector<unsigned int> indices;
 vertices.reserve((lonSteps+1)*(latSteps+1));
 indices.reserve(lonSteps*latSteps*6);
 for(int j=0;j<=latSteps;++j){
  double lat=(-90.0+180.0*j/latSteps)*toRadians;
  double cl=std::cos(lat),sl=std::sin(lat);
  for(int i=0;i<=lonSteps;++i){
   double lon=(-180.0+360.0*i/lonSteps)*toRadians;
   float x=float(cl*std::cos(lon)),y=float(sl),z=float(cl*std::sin(lon));
   vertices.push_back({x,y,z,x,y,z,float(i)/lonSteps,float(j)/latSteps});
  }
 }
 for(int j=0;j<latSteps;++j)
  for(int i=0;i<lonSteps;++i){
   unsigned a=j*(lonSteps+1)+i,b=a+(lonSteps+1);
   indices.insert(indices.end(),{a,b,b+1,a,b+1,a+1});
  }
 indexCount_=int(indices.size());
 vao_.create();vao_.bind();
 vertices_.create();vertices_.bind();vertices_.allocate(vertices.data(),int(vertices.size()*sizeof(Vertex)));
 indices_.create();indices_.bind();indices_.allocate(indices.data(),int(indices.size()*sizeof(unsigned int)));
 shader_.bind();
 shader_.enableAttributeArray(0);shader_.setAttributeBuffer(0,GL_FLOAT,offsetof(Vertex,px),3,sizeof(Vertex));
 shader_.enableAttributeArray(1);shader_.setAttributeBuffer(1,GL_FLOAT,offsetof(Vertex,nx),3,sizeof(Vertex));
 shader_.enableAttributeArray(2);shader_.setAttributeBuffer(2,GL_FLOAT,offsetof(Vertex,u),2,sizeof(Vertex));
 shader_.release();
 vao_.release();indices_.release();vertices_.release();
}
bool EarthViewport::uploadImageTexture(const QString& path,GLuint& target){
 QImage image(path);
 if(image.isNull()||image.size()!=QSize(2048,1024))return false;
 image=image.convertToFormat(QImage::Format_RGBA8888).mirrored(false,true);
 if(target){glDeleteTextures(1,&target);target=0;}
 glGenTextures(1,&target);glBindTexture(GL_TEXTURE_2D,target);
 glPixelStorei(GL_UNPACK_ALIGNMENT,1);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,image.width(),image.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,image.constBits());
 glGenerateMipmap(GL_TEXTURE_2D);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 glBindTexture(GL_TEXTURE_2D,0);
 return true;
}
bool EarthViewport::reloadTexture(){
 return glInitialized_&&!texturePath_.isEmpty()&&uploadImageTexture(texturePath_,texture_);
}
bool EarthViewport::reloadMoonTexture(){
 return glInitialized_&&!moonTexturePath_.isEmpty()&&uploadImageTexture(moonTexturePath_,moonTexture_);
}
void EarthViewport::resizeGL(int w,int h){glViewport(0,0,w,h);}
void EarthViewport::paintGL(){
 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 if(!glInitialized_)return;
 float a=yaw_*float(toRadians),e=pitch_*float(toRadians);
 QVector3D eye(radius_*std::cos(e)*std::cos(a),radius_*std::sin(e),radius_*std::cos(e)*std::sin(a));
 QMatrix4x4 model,view,proj;
 // Qt rotates positive Y from X to -Z, hence negative GMST.
 if(scene_==Scene::Earth)model.rotate(float(-solar_.greenwichMeanSiderealDeg),0,1,0);
 view.lookAt(eye,QVector3D(0,0,0),QVector3D(0,1,0));
 proj.perspective(38.0f,float(width())/float(std::max(1,height())),0.01f,90.0f);
 shader_.bind();
 shader_.setUniformValue("uModel",model);shader_.setUniformValue("uView",view);
 shader_.setUniformValue("uProjection",proj);
 const QVector3D light=scene_==Scene::Earth ? sunWorld_ : QVector3D(0.8f,0.45f,0.6f).normalized();
 shader_.setUniformValue("uSunWorld",light);
 const GLuint currentTexture=scene_==Scene::Earth?texture_:moonTexture_;
 shader_.setUniformValue("uCameraWorld",eye);shader_.setUniformValue("uHasTexture",currentTexture!=0);
 shader_.setUniformValue("uIsMoon",scene_==Scene::Moon);
 shader_.setUniformValue("uEarthTexture",0);
 glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,currentTexture);
 vao_.bind();glDrawElements(GL_TRIANGLES,indexCount_,GL_UNSIGNED_INT,nullptr);
 vao_.release();glBindTexture(GL_TEXTURE_2D,0);shader_.release();
 QPainter painter(this);painter.setRenderHint(QPainter::TextAntialiasing);
 painter.setPen(QColor("#90a6c1"));painter.setFont(QFont("Segoe UI",10));
 painter.drawText(QRect(20,18,width()-40,50),Qt::AlignLeft|Qt::AlignTop,
                  scene_==Scene::Earth?
  "EARTH / APPROXIMATE SOLAR GEOMETRY\n拖动旋转视角 · 滚轮缩放":
  "MOON / NASA LRO GLOBAL COLOR MOSAIC\n资料地球仪 · 固定近侧朝向 · 展示光源（非实时月相）");
 if(!hasObservedTexture()){
  painter.setPen(QColor("#ffc99c"));
  painter.drawText(QRect(12,height()-75,width()-24,60),Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap,
                   scene_==Scene::Earth?
   "NASA Blue Marble 真实影像尚未安装\n当前灰色球体仅为几何占位，非地球照片":
   "NASA LRO 月球全球影像尚未安装\n当前灰色球体仅为几何占位，非月球照片");
 }
}
void EarthViewport::mousePressEvent(QMouseEvent* e){lastMouse_=e->position().toPoint();}
void EarthViewport::mouseMoveEvent(QMouseEvent* e){
 if(e->buttons()&Qt::LeftButton){
  QPoint now=e->position().toPoint(),delta=now-lastMouse_;
  yaw_+=delta.x()*0.35f;pitch_=std::clamp(pitch_-delta.y()*0.35f,-85.0f,85.0f);
  lastMouse_=now;update();
 }
}
void EarthViewport::wheelEvent(QWheelEvent* e){
 float steps=float(e->angleDelta().y())/120.0f;
 radius_=std::clamp(radius_*std::pow(0.87f,steps),1.6f,30.0f);update();
}
} // namespace atlas
