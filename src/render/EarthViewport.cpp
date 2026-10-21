#include "render/EarthViewport.hpp"
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QOpenGLContext>
#include <QVector4D>
#include <exception>
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
void EarthViewport::setMoonBodyFrame(const astro::LunarBodyFrame& frame){
 // MOON_ME +X=0E, +Y=90E, +Z=north. Our mesh uses +X, +Z, +Y.
 moonSunMeScene_=QVector3D(float(frame.moonToSunMe[0]),float(frame.moonToSunMe[2]),float(frame.moonToSunMe[1])).normalized();
 moonEarthMeScene_=QVector3D(float(frame.moonToEarthMe[0]),float(frame.moonToEarthMe[2]),float(frame.moonToEarthMe[1])).normalized();
 preciseMoonFrame_=true;
 if(scene_==Scene::Moon&&followSubEarth_)focusSubEarth();
 update();
}
void EarthViewport::clearMoonBodyFrame(){preciseMoonFrame_=false;update();}
void EarthViewport::focusSubEarth(){
 if(scene_!=Scene::Moon)return;
 if(!preciseMoonFrame_){yaw_=0.0f;pitch_=12.0f;}
 else{
  constexpr float deg=180.0f/3.14159265358979323846f;
  yaw_=std::atan2(moonEarthMeScene_.z(),moonEarthMeScene_.x())*deg;
  pitch_=std::asin(std::clamp(moonEarthMeScene_.y(),-1.0f,1.0f))*deg;
 }
 radius_=3.65f;
 followSubEarth_=true;
 update();
}
void EarthViewport::updateSystemCamera(){
 if(scene_!=Scene::EarthMoon||!systemValid_)return;
 if(systemCameraMode_==1){cameraTarget_=QVector3D{};return;}
 if(systemCameraMode_==2){cameraTarget_=moonCenterScene_;return;}
 cameraTarget_=moonCenterScene_*0.5f;
 const auto direction=moonCenterScene_.normalized();
 QVector3D side=QVector3D::crossProduct(direction,QVector3D(0,1,0));
 if(side.lengthSquared()<0.01f)side=QVector3D::crossProduct(direction,QVector3D(0,0,1));
 side.normalize();
 constexpr float deg=180.0f/3.14159265358979323846f;
 yaw_=std::atan2(side.z(),side.x())*deg;
 pitch_=19.0f;
 radius_=std::max(9.0f,moonCenterScene_.length()*2.2f);
}
void EarthViewport::focusSystemOverview(){
 systemCameraMode_=0;updateSystemCamera();update();
}
void EarthViewport::focusSystemEarth(){
 if(scene_!=Scene::EarthMoon)return;
 systemCameraMode_=1;cameraTarget_=QVector3D{};radius_=3.85f;update();
}
void EarthViewport::focusSystemMoon(){
 if(scene_!=Scene::EarthMoon||!systemValid_)return;
 systemCameraMode_=2;cameraTarget_=moonCenterScene_;radius_=1.4f;update();
}
void EarthViewport::setDistanceCompression(double factor){
 if(factor!=1.0&&factor!=12.0)return;
 distanceCompression_=factor;
 if(systemValid_){
  systemLayout_=astro::earthMoonLayout(trueMoonJ2000Km_,distanceCompression_);
  const auto& center=systemLayout_.moonCenterJ2000EarthRadii;
  moonCenterScene_=QVector3D(float(center[0]),float(center[2]),float(center[1]));
  if(scene_==Scene::EarthMoon){updateSystemCamera();update();}
 }
}
void EarthViewport::setEarthMoonSystem(const astro::SolarLunarSample& data,
 const astro::LunarBodyFrame& body){
 try{systemLayout_=astro::earthMoonLayout(data.earthToMoonJ2000Km,distanceCompression_);}
 catch(const std::exception&){clearEarthMoonSystem();return;}
 trueMoonJ2000Km_=data.earthToMoonJ2000Km;
 const auto& center=systemLayout_.moonCenterJ2000EarthRadii;
 moonCenterScene_=QVector3D(float(center[0]),float(center[2]),float(center[1]));
 const auto& r=body.meToJ2000;
 moonMeToJ2000Scene_.setToIdentity();
 // Render axes are NAIF (X,Z,Y). Apply the same permutation to both
 // the J2000 and MOON_ME basis: P * R * P^-1.
 moonMeToJ2000Scene_.setRow(0,QVector4D(float(r[0]),float(r[2]),float(r[1]),0));
 moonMeToJ2000Scene_.setRow(1,QVector4D(float(r[6]),float(r[8]),float(r[7]),0));
 moonMeToJ2000Scene_.setRow(2,QVector4D(float(r[3]),float(r[5]),float(r[4]),0));
 const auto& sun=data.earthToSunJ2000Km;
 const auto& moon=data.earthToMoonJ2000Km;
 sunInertialScene_=QVector3D(float(sun[0]),float(sun[2]),float(sun[1])).normalized();
 moonSunInertialScene_=QVector3D(float(sun[0]-moon[0]),float(sun[2]-moon[2]),
                               float(sun[1]-moon[1])).normalized();
 systemValid_=true;
 updateSystemCamera();update();
}
void EarthViewport::clearEarthMoonSystem(){systemValid_=false;update();}
void EarthViewport::setScene(Scene scene){
 scene_=scene;
 if(scene_==Scene::Moon){followSubEarth_=true;focusSubEarth();}
 else if(scene_==Scene::EarthMoon){focusSystemOverview();}
 else{resetCamera();}
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
 if(scene_==Scene::Moon){focusSubEarth();return;}
 if(scene_==Scene::EarthMoon){focusSystemOverview();return;}
 const float pi=3.14159265358979323846f;
 yaw_=std::atan2(sunWorld_.z(),sunWorld_.x())*180.0f/pi;
 pitch_=std::asin(std::clamp(sunWorld_.y(),-1.0f,1.0f))*180.0f/pi;
 radius_=3.85f;
 update();
}
void EarthViewport::resetCamera(){
 if(scene_==Scene::EarthMoon){focusSystemOverview();return;}
 yaw_=39.0f;pitch_=19.0f;radius_=3.3f;update();
}
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
 const float azimuth=yaw_*float(toRadians),elevation=pitch_*float(toRadians);
 const QVector3D offset(radius_*std::cos(elevation)*std::cos(azimuth),
                         radius_*std::sin(elevation),
                         radius_*std::cos(elevation)*std::sin(azimuth));
 const bool system=scene_==Scene::EarthMoon;
 const QVector3D target=system?cameraTarget_:QVector3D{};
 const QVector3D eye=target+offset;
 QMatrix4x4 view,projection;
 view.lookAt(eye,target,QVector3D(0,1,0));
 projection.perspective(38.0f,float(width())/float(std::max(1,height())),0.01f,1000.0f);
 const auto drawBody=[&](const QMatrix4x4& model,GLuint tex,bool isMoon,
                         const QVector3D& sunlight,bool preciseLighting){
  shader_.bind();
  shader_.setUniformValue("uModel",model);
  shader_.setUniformValue("uView",view);
  shader_.setUniformValue("uProjection",projection);
  shader_.setUniformValue("uSunWorld",sunlight);
  shader_.setUniformValue("uCameraWorld",eye);
  shader_.setUniformValue("uHasTexture",tex!=0);
  shader_.setUniformValue("uIsMoon",isMoon);
  shader_.setUniformValue("uPreciseMoonLighting",preciseLighting);
  shader_.setUniformValue("uEarthTexture",0);
  glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,tex);
  vao_.bind();glDrawElements(GL_TRIANGLES,indexCount_,GL_UNSIGNED_INT,nullptr);
  vao_.release();glBindTexture(GL_TEXTURE_2D,0);shader_.release();
 };
 QMatrix4x4 earthModel;
 earthModel.rotate(float(-solar_.greenwichMeanSiderealDeg),0,1,0);
 if(system){
  if(systemValid_){
   drawBody(earthModel,texture_,false,sunInertialScene_,false);
   QMatrix4x4 moonModel;
   moonModel.translate(moonCenterScene_);
   moonModel*=moonMeToJ2000Scene_;
   moonModel.scale(float(astro::moonRadiusEarthUnits));
   drawBody(moonModel,moonTexture_,true,moonSunInertialScene_,true);
  }
 }else if(scene_==Scene::Moon){
  QMatrix4x4 moonModel;
  const QVector3D light=preciseMoonFrame_?moonSunMeScene_:
   QVector3D(0.8f,0.45f,0.6f).normalized();
  drawBody(moonModel,moonTexture_,true,light,preciseMoonFrame_);
 }else drawBody(earthModel,texture_,false,sunWorld_,false);

 QPainter painter(this);
 painter.setRenderHint(QPainter::TextAntialiasing);
 painter.setPen(QColor("#90a6c1"));
 painter.setFont(QFont("Segoe UI",10));
 const QString caption=system?
   (systemValid_?
    QStringLiteral("EARTH-MOON / JPL GEOMETRIC J2000\n真实中心距离 %1 km · %2 · 半径保持真实比")
     .arg(systemLayout_.trueDistanceKm,0,'f',0)
     .arg(distanceCompression_==1.0?QStringLiteral("真实比例 1:1"):
                                       QStringLiteral("中心距离压缩 12 倍（非物理比例）")):
    QStringLiteral("EARTH-MOON DATA UNAVAILABLE\n需要 CSPICE DE442s + DE440 月球 PCK/FK；不绘制假轨道")):
   scene_==Scene::Earth?
    QStringLiteral("EARTH / APPROXIMATE SOLAR GEOMETRY\n拖动旋转视角 · 滚轮缩放"):
    preciseMoonFrame_?
      QStringLiteral("MOON / JPL DE440 MOON_ME + DE442s\n真实 NASA LROC 地图 · 地心几何月相"):
      QStringLiteral("MOON / NASA LRO SURVEY\n固定展示光源（非真实月相）");
 painter.drawText(QRect(20,18,width()-40,58),Qt::AlignLeft|Qt::AlignTop,caption);
 if(!hasObservedTexture()){
  painter.setPen(QColor("#ffc99c"));
  const QString warning=system?
   (systemValid_?QStringLiteral("NASA 地球与月球两套真实全球影像尚不齐全"):
                 QStringLiteral("没有官方星历或姿态内核；不绘制虚构轨道")):
   scene_==Scene::Moon?QStringLiteral("NASA LRO 月面地图未安装；当前为几何占位"):
                       QStringLiteral("NASA Blue Marble 地球地图未安装；当前为几何占位");
  painter.drawText(QRect(12,height()-90,width()-24,70),
   Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap,warning);
 }
}
void EarthViewport::mousePressEvent(QMouseEvent* e){lastMouse_=e->position().toPoint();}
void EarthViewport::mouseMoveEvent(QMouseEvent* e){
 if(e->buttons()&Qt::LeftButton){
  QPoint now=e->position().toPoint(),delta=now-lastMouse_;
  yaw_+=delta.x()*0.35f;pitch_=std::clamp(pitch_-delta.y()*0.35f,-85.0f,85.0f);
  lastMouse_=now;if(scene_==Scene::Moon)followSubEarth_=false;update();
 }
}
void EarthViewport::wheelEvent(QWheelEvent* e){
 float steps=float(e->angleDelta().y())/120.0f;
 const float lower=scene_==Scene::EarthMoon?0.7f:1.6f;
 radius_=std::clamp(radius_*std::pow(0.87f,steps),lower,500.0f);update();
}
} // namespace atlas
