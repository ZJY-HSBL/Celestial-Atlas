#include "app/MainWindow.hpp"
#include "astronomy/SolarGeometry.hpp"
#include "render/EarthViewport.hpp"
#include <QComboBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>
#include <QSaveFile>
#include <QScrollArea>
#include <QStyle>
#include <QFrame>
#include <QDateTime>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QFont>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QWidget>
#include <chrono>
namespace atlas {
namespace {
QLabel* label(const QString& text,const char* name=nullptr){
 auto* l=new QLabel(text);
 if(name)l->setObjectName(QString::fromLatin1(name));
 l->setWordWrap(true);
 return l;
}
}
MainWindow::MainWindow(QWidget* parent):QMainWindow(parent),downloader_(this),moonDownloader_(this,TextureDownloader::Body::Moon){
 setWindowTitle(QStringLiteral("Celestial Atlas | 科学宇宙探索"));
 resize(1240,790);setMinimumSize(860,540);
 auto* page=new QWidget(this);
 auto* root=new QVBoxLayout(page);
 root->setContentsMargins(14,14,14,14);root->setSpacing(12);
 auto* header=new QHBoxLayout;
 headerTitle_=label(QStringLiteral("✦  CELESTIAL ATLAS    /    地球科学观测"),"heading");
 header->addWidget(headerTitle_);
 header->addStretch();
 auto* sceneChoice=new QComboBox;
 sceneSelector_=sceneChoice;
 sceneChoice->addItem(QStringLiteral("地球 / Earth"));
 sceneChoice->addItem(QStringLiteral("月球 / Moon Survey"));
 sceneChoice->setToolTip(QStringLiteral("已安装 CSPICE + DE440 PCK/FK 时采用真实天平动与太阳几何方位，否则固定测绘灯光"));
 header->addWidget(sceneChoice);
 downloadButton_=new QPushButton(QStringLiteral("下载 NASA 地球真实影像"));
 header->addWidget(downloadButton_);
 root->addLayout(header);
 auto* workspace=new QHBoxLayout;
 workspace->setSpacing(10);
 auto* left=new QWidget;
 left->setObjectName("sidePanel");
 left->setFixedWidth(174);
 auto* leftLayout=new QVBoxLayout(left);
 leftLayout->setContentsMargins(12,15,12,15);
 leftLayout->setSpacing(12);
 leftLayout->addWidget(label(QStringLiteral("EXPLORATION"),"section"));
 earthNav_=label(QStringLiteral("●  地球空间视图"),"activeItem");
 moonNav_=label(QStringLiteral("◌  月球全球测绘"),"planned");
 leftLayout->addWidget(earthNav_);
 leftLayout->addWidget(moonNav_);
 leftLayout->addWidget(label(QStringLiteral("◌  太阳系导航\n     下一阶段"),"planned"));
 leftLayout->addWidget(label(QStringLiteral("✧  地面真实星空\n     下一阶段"),"planned"));
 leftLayout->addWidget(label(QStringLiteral("◇  恒星际导航\n     后续阶段"),"planned"));
 leftLayout->addStretch();
 leftLayout->addWidget(label(QStringLiteral("DATA STATUS"),"section"));
 textureInfo_=label(QStringLiteral("尚未安装 NASA 全球影像"),"detail");
 leftLayout->addWidget(textureInfo_);
 workspace->addWidget(left);
 viewport_=new EarthViewport;
 viewport_->setObjectName("viewport");
 workspace->addWidget(viewport_,1);
 auto* right=new QWidget;
 right->setObjectName("sidePanel");right->setFixedWidth(270);
 auto* rightLayout=new QVBoxLayout(right);
 rightLayout->setContentsMargins(16,18,16,18);rightLayout->setSpacing(12);
 rightLayout->addWidget(label(QStringLiteral("SELECTED BODY"),"section"));
 selectedName_=label(QStringLiteral("地球  /  EARTH"),"title");
 rightLayout->addWidget(selectedName_);
 selectedSubtitle_=label(QStringLiteral("太阳系 · 第三颗行星"),"detail");
 rightLayout->addWidget(selectedSubtitle_);
 rightLayout->addSpacing(12);
 selectedRadius_=label(QStringLiteral("平均半径    6,371 km"),"data");
 rightLayout->addWidget(selectedRadius_);
 solarHeader_=label(QStringLiteral("太阳光照与地球旋转"),"section");
 rightLayout->addWidget(solarHeader_);
 solarInfo_=label(QStringLiteral("正在计算..."),"data");
 rightLayout->addWidget(solarInfo_);
 rightLayout->addWidget(label(QStringLiteral("MOON / 地月系统"),"section"));
 moonInfo_=label(QStringLiteral("精密月相尚未加载"),"data");
 rightLayout->addWidget(moonInfo_);
 lunarMapStatus_=label(QStringLiteral("月球全球 LRO 影像未安装"),"detail");
 rightLayout->addWidget(lunarMapStatus_);
 auto* moonPhoto=new QPushButton(QStringLiteral("NASA LRO 月球真实观测影像"));
 connect(moonPhoto,&QPushButton::clicked,this,[]{
  QDesktopServices::openUrl(QUrl(QStringLiteral("https://science.nasa.gov/resource/moon-mosaic/")));
 });
 rightLayout->addWidget(moonPhoto);
 auto* downloadMoon=new QPushButton(QStringLiteral("下载 NASA LRO 全球月面影像"));
 connect(downloadMoon,&QPushButton::clicked,&moonDownloader_,&TextureDownloader::download);
 rightLayout->addWidget(downloadMoon);
 connect(&moonDownloader_,&TextureDownloader::ready,this,[this,downloadMoon](const QString& path,const QString& hash){
  downloadMoon->setText(QStringLiteral("更新 NASA LRO 月面影像"));
  if(viewport_->setMoonTextureFile(path)){
   lunarMapStatus_->setText(QStringLiteral("NASA LROC 2048×1024\nSHA-256: %1…").arg(hash.left(12)));
  }else lunarMapStatus_->setText(QStringLiteral("LRO 影像下载成功但 OpenGL 加载失败"));
 });
 connect(&moonDownloader_,&TextureDownloader::failed,this,[this](const QString& error){
  lunarMapStatus_->setText(error);
 });
 rightLayout->addStretch();
 rightLayout->addWidget(label(QStringLiteral(
  "SCIENCE DISCLOSURE\n"
  "太阳方向：低精度近似算法（尚非 JPL 精密星历）\n"
  "地球影像：NASA 多日期卫星拼接，非实时云图\n"
  "大气光照：可视化近似，不是辐射传输模型"),"caution"));
 auto* focus=new QPushButton(QStringLiteral("聚焦当前地球白昼面"));
 connect(focus,&QPushButton::clicked,viewport_,&EarthViewport::focusDaylight);
 rightLayout->addWidget(focus);
 connect(sceneChoice,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this,focus](int value){
  const bool moon=value==1;
  viewport_->setScene(moon?EarthViewport::Scene::Moon:EarthViewport::Scene::Earth);
  selectedName_->setText(moon?QStringLiteral("月球  /  MOON"):QStringLiteral("地球  /  EARTH"));
  selectedSubtitle_->setText(moon?QStringLiteral("天然卫星 · NASA LRO 全球测绘影像"):QStringLiteral("太阳系 · 第三颗行星"));
  selectedRadius_->setText(moon?QStringLiteral("平均半径    1,737.4 km"):QStringLiteral("平均半径    6,371 km"));
  headerTitle_->setText(moon?QStringLiteral("✦  CELESTIAL ATLAS   /   月球全球测绘"):QStringLiteral("✦  CELESTIAL ATLAS   /   地球科学观测"));
  solarHeader_->setVisible(!moon);
  solarInfo_->setVisible(!moon);
  earthNav_->setObjectName(moon?"planned":"activeItem");
  moonNav_->setObjectName(moon?"activeItem":"planned");
  for(QLabel* nav:{earthNav_,moonNav_}){
   nav->style()->unpolish(nav);nav->style()->polish(nav);nav->update();
  }
  downloadButton_->setText(moon?QStringLiteral("下载 NASA LRO 月面影像"):QStringLiteral("下载 NASA 地球真实影像"));
  focus->setText(moon?QStringLiteral("回到地心视向 / 重置视角"):QStringLiteral("聚焦当前地球白昼面"));
  textureInfo_->setText(moon?QStringLiteral("LRO 月面地图：完整全球观测拼接，极区补充资料"):QStringLiteral("NASA Blue Marble 真实地球资料"));
 });
 auto* snapshot=new QPushButton(QStringLiteral("保存真实渲染截图"));
 connect(snapshot,&QPushButton::clicked,this,[this]{
  if(!viewport_->hasObservedTexture()){
   QMessageBox::warning(this,QStringLiteral("影像未加载"),
    QStringLiteral("请先下载 NASA Blue Marble 真实卫星拼接影像，不能把无纹理占位画面当成真实地球截图。"));
   return;
  }
  const QString path=QFileDialog::getSaveFileName(this,QStringLiteral("保存真实运行截图"),
    QStringLiteral("CelestialAtlas-Earth.png"),QStringLiteral("PNG 图像 (*.png)"));
  if(path.isEmpty())return;
  if(!captureViewport(path)){
   QMessageBox::warning(this,QStringLiteral("截图失败"),QStringLiteral("OpenGL 视口截图失败，请检查显卡驱动和路径。"));
  }else{
   QMessageBox::information(this,QStringLiteral("截图已保存"),
    QStringLiteral("真实程序截图与科学来源元数据已保存。"));
  }
 });
 rightLayout->addWidget(snapshot);
 auto* workstationShot=new QPushButton(QStringLiteral("保存完整工作台截图"));
 connect(workstationShot,&QPushButton::clicked,this,[this]{
  if(!viewport_->hasObservedTexture()){
   QMessageBox::warning(this,QStringLiteral("影像未加载"),QStringLiteral("请先加载 NASA 真实影像。"));return;
  }
  const QString path=QFileDialog::getSaveFileName(this,QStringLiteral("保存完整工作台截图"),
   QStringLiteral("CelestialAtlas-Workbench.png"),QStringLiteral("PNG 图像 (*.png)"));
  if(path.isEmpty())return;
  if(!captureWorkbench(path)){
   QMessageBox::warning(this,QStringLiteral("截图失败"),QStringLiteral("未能生成真实工作台截图。"));
  }
 });
 rightLayout->addWidget(workstationShot);
 auto* reset=new QPushButton(QStringLiteral("重置观察视角"));
 connect(reset,&QPushButton::clicked,viewport_,&EarthViewport::resetCamera);
 rightLayout->addWidget(reset);
 auto* rightScroll=new QScrollArea;
 rightScroll->setFrameShape(QFrame::NoFrame);
 rightScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 rightScroll->setWidgetResizable(true);
 rightScroll->setFixedWidth(289);
 rightScroll->setWidget(right);
 workspace->addWidget(rightScroll);
 root->addLayout(workspace,1);
 auto* footer=new QHBoxLayout;
 footer->setSpacing(12);
 footer->addWidget(label(QStringLiteral("TIME / UTC"),"section"));
 timeEdit_=new QDateTimeEdit(QDateTime::currentDateTimeUtc());
 timeEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss 'UTC'"));
 timeEdit_->setCalendarPopup(true);
 timeEdit_->setMinimumDateTime(QDateTime(QDate(1990,1,1),QTime(0,0),Qt::UTC));
 timeEdit_->setMaximumDateTime(QDateTime(QDate(2050,12,31),QTime(23,59),Qt::UTC));
 footer->addWidget(timeEdit_);
 auto* now=new QPushButton(QStringLiteral("当前时间"));
 footer->addWidget(now);
 auto* speed=new QComboBox;
 speedSelector_=speed;
 speed->addItem(QStringLiteral("暂停"),0.0);
 speed->addItem(QStringLiteral("1 × 真实时间"),1.0);
 speed->addItem(QStringLiteral("60 ×"),60.0);
 speed->addItem(QStringLiteral("3600 ×"),3600.0);
 speed->setCurrentIndex(1);footer->addWidget(speed);
 footer->addStretch();
 status_=label(QStringLiteral("本地科学演示 V0.5.0"),"detail");
 footer->addWidget(status_);
 root->addLayout(footer);
 setCentralWidget(page);
 setStyleSheet(R"QSS(
 QWidget { background:#0b101a; color:#dfe8f4; font-family:'Microsoft YaHei UI','Segoe UI'; font-size:13px; }
 QWidget#sidePanel { background:#131d2a; border:1px solid #28384a; border-radius:10px; }
 QWidget#viewport { border:1px solid #25394e; }
 QLabel#heading { color:#f1f7ff; font-size:18px; font-weight:700; letter-spacing:1px; }
 QLabel#title { color:#f2f7fe; font-size:21px; font-weight:700; }
 QLabel#section { color:#8da4c2; font-size:10px; font-weight:700; }
 QLabel#activeItem { color:#bde0ff; background:#23384d; border-radius:5px; padding:10px; }
 QLabel#planned { color:#768da7; padding:7px; }
 QLabel#detail { color:#a5b7cf; font-size:11px; }
 QLabel#data { color:#e0eaf6; font-size:12px; }
 QLabel#caution { color:#aeb9c6; font-size:11px; background:#1b2735; border-radius:5px; padding:10px; }
 QPushButton,QComboBox,QDateTimeEdit { background:#1f3044; border:1px solid #35506a; border-radius:6px; padding:9px 12px; color:#e5f0ff; }
 QPushButton:hover { background:#304865; }
 QPushButton:disabled { color:#8293a9; }
 QComboBox QAbstractItemView { background:#1f3044; color:#edf3ff; }
 )QSS");
 simulatedMilliseconds_=double(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
 connect(timeEdit_,&QDateTimeEdit::dateTimeChanged,this,[this](const QDateTime& date){
  simulatedMilliseconds_=double(date.toMSecsSinceEpoch());
 });
 connect(now,&QPushButton::clicked,this,[this]{
  simulatedMilliseconds_=double(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
  refreshStatus();
 });
 connect(speed,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this,speed](int i){
  simulationSpeed_=speed->itemData(i).toDouble();
 });
 connect(downloadButton_,&QPushButton::clicked,this,[this]{
  if(viewport_->scene()==EarthViewport::Scene::Moon)moonDownloader_.download();
  else downloader_.download();
 });
 connect(&downloader_,&TextureDownloader::ready,this,&MainWindow::setImageReady);
 connect(&downloader_,&TextureDownloader::failed,this,[this](const QString& error){
  downloadButton_->setEnabled(true);
  downloadButton_->setText(QStringLiteral("重试 NASA 影像下载"));
  textureInfo_->setText(error);
 });
 connect(&downloader_,&TextureDownloader::progress,this,[this](qint64 received,qint64 total){
  if(total>0)downloadButton_->setText(QStringLiteral("影像下载 %1%").arg(100*received/total));
 });
 const QString lunarCache=TextureDownloader::moonExpectedImagePath();
 if(QFileInfo::exists(lunarCache)&&viewport_->setMoonTextureFile(lunarCache))
  lunarMapStatus_->setText(QStringLiteral("NASA LROC 2K 月球全球影像已缓存"));
 const QString cached=TextureDownloader::expectedImagePath();
 if(QFileInfo::exists(cached)&&viewport_->setTextureFile(cached)){
  textureInfo_->setText(QStringLiteral("NASA Blue Marble 2048 × 1024\n本地缓存已加载"));
  downloadButton_->setText(QStringLiteral("更新 NASA 影像"));
 }
 connect(&frameTimer_,&QTimer::timeout,this,&MainWindow::tick);
 {
  QString kernelDir=qEnvironmentVariable("ATLAS_SPICE_KERNEL_DIR");
  if(kernelDir.isEmpty())
   kernelDir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/kernels");
  std::string kernelError;
  if(spice_.loadKernels(QDir(kernelDir).filePath(QStringLiteral("naif0012.tls")).toStdString(),
      QDir(kernelDir).filePath(QStringLiteral("de442s.bsp")).toStdString(),kernelError)){
   moonInfo_->setText(QStringLiteral("JPL DE442s 精密星历已加载"));
   std::string frameError;
   if(spice_.loadMoonOrientation(QDir(kernelDir).filePath(QStringLiteral("moon_de440_250416.tf")).toStdString(),
       QDir(kernelDir).filePath(QStringLiteral("moon_pa_de440_200625.bpc")).toStdString(),frameError)){
    moonInfo_->setText(QStringLiteral("JPL DE440 MOON_ME 月球姿态已加载"));
   }else{
    moonInfo_->setText(QStringLiteral("JPL DE442s 轨道可用；缺少 DE440 FK/PCK，使用测绘浏览模式"));
   }
  } else if(astro::SpiceEphemeris::builtWithSpice()){
   moonInfo_->setText(QStringLiteral("需要 NASA 官方 naif0012.tls / de442s.bsp 内核。\n当前月相不可用。"));
  }else{
   moonInfo_->setText(QStringLiteral("此 Windows 预览包未链接 CSPICE。\n未提供月相数值，避免虚构数据。"));
  }
 }
 frameTimer_.setInterval(16);elapsed_.start();frameTimer_.start();refreshStatus();
}
void MainWindow::tick(){
 const qint64 dt=elapsed_.restart();
 simulatedMilliseconds_+=qMin(dt,qint64(250))*simulationSpeed_;
 using namespace std::chrono;
 const auto timestamp=std::chrono::system_clock::time_point(milliseconds(qint64(simulatedMilliseconds_)));
 viewport_->setSolarGeometry(astro::solarGeometry(timestamp));
 if(++frames_>=8){frames_=0;refreshStatus();}
}
void MainWindow::refreshStatus(){
 const auto utc=QDateTime::fromMSecsSinceEpoch(qint64(simulatedMilliseconds_),Qt::UTC);
 {QSignalBlocker guard(timeEdit_);timeEdit_->setDateTime(utc);}
 using namespace std::chrono;
 auto timestamp=system_clock::time_point(milliseconds(utc.toMSecsSinceEpoch()));
 const auto sun=astro::solarGeometry(timestamp);
 solarInfo_->setText(QStringLiteral(
  "太阳赤经       %1°\n"
  "太阳赤纬       %2°\n"
  "格林尼治恒星时   %3°\n"
  "太阳直射纬度    %4°\n"
  "太阳直射经度    %5°")
 .arg(sun.rightAscensionDeg,0,'f',3)
 .arg(sun.declinationDeg,0,'f',3)
 .arg(sun.greenwichMeanSiderealDeg,0,'f',3)
 .arg(sun.subsolarLatitudeDeg,0,'f',3)
 .arg(sun.subsolarLongitudeDeg,0,'f',3));
 if(spice_.ready()){
  astro::SolarLunarSample result;
  std::string error;
  if(spice_.sample(timestamp,result,error)){
   moonInfo_->setText(QStringLiteral(
    "数据：JPL DE442s / NAIF CSPICE\n"
    "月面照明比例  %1%\n"
    "地月距离      %2 km\n"
    "日地距离      %3 km\n"
    "月球相位角     %4°\n"
    "参考系 J2000 / 几何位置，未修正光行时")
    .arg(result.lunar.illuminatedFraction*100.0,0,'f',2)
    .arg(result.lunar.moonDistanceKm,0,'f',0)
    .arg(result.lunar.sunDistanceKm,0,'f',0)
    .arg(result.lunar.phaseAngleDeg,0,'f',2));
  }else{
   moonInfo_->setText(QStringLiteral("JPL 星历计算失败：%1").arg(QString::fromStdString(error)));
  }
  if(spice_.lunarOrientationReady()){
   astro::LunarBodyFrame bodyFrame;
   if(spice_.sampleMoonBodyFrame(timestamp,bodyFrame,error)){
    viewport_->setMoonBodyFrame(bodyFrame);
    moonInfo_->setText(moonInfo_->text()+
     QStringLiteral("\n月球固连 ME 坐标\n地球下点经度 %1° / 纬度 %2°\n太阳下点经度 %3° / 纬度 %4°")
       .arg(bodyFrame.subEarthLongitudeDeg,0,'f',3)
       .arg(bodyFrame.subEarthLatitudeDeg,0,'f',3)
       .arg(bodyFrame.subSolarLongitudeDeg,0,'f',3)
       .arg(bodyFrame.subSolarLatitudeDeg,0,'f',3));
   }else{
    viewport_->clearMoonBodyFrame();
    moonInfo_->setText(QStringLiteral("月球姿态计算失败: %1").arg(QString::fromStdString(error)));
   }
  }
 }
}
void MainWindow::focusDaylight(){viewport_->focusDaylight();}
void MainWindow::setSimulationTimeUtc(const QDateTime& utc){
 simulatedMilliseconds_=double(utc.toUTC().toMSecsSinceEpoch());
 speedSelector_->setCurrentIndex(0);
 refreshStatus();
}
bool MainWindow::loadMoonTextureFromPath(const QString& path){
 const bool ok=viewport_->setMoonTextureFile(path);
 if(ok)lunarMapStatus_->setText(QStringLiteral("NASA LROC 2048×1024 全球月面测绘影像已加载"));
 return ok;
}
void MainWindow::selectMoonScene(){sceneSelector_->setCurrentIndex(1);}
bool MainWindow::loadTextureFromPath(const QString& path){
 if(!viewport_->setTextureFile(path))return false;
 textureInfo_->setText(QStringLiteral("NASA Blue Marble 2048 × 1024\n真实观测拼接影像"));
 return true;
}
bool MainWindow::captureWorkbench(const QString& outputPath){
 if(!viewport_->isValid()||!viewport_->hasObservedTexture())return false;
 const QPixmap pixels=grab();
 if(pixels.isNull()||pixels.width()<300||pixels.height()<300||!pixels.save(outputPath,"PNG"))return false;
 QJsonObject provenance{
  {"application","Celestial Atlas 0.5.0"},
  {"capture_kind","Full actual Qt application window including native OpenGL viewport"},
  {"simulated_utc",QDateTime::fromMSecsSinceEpoch(qint64(simulatedMilliseconds_),Qt::UTC).toString(Qt::ISODate)},
  {"earth_texture_source","https://svs.gsfc.nasa.gov/2915/"},
  {"rendered_body",viewport_->scene()==EarthViewport::Scene::Moon?"Moon":"Earth"},
  {"actual_render_texture",viewport_->scene()==EarthViewport::Scene::Moon?
   "https://svs.gsfc.nasa.gov/4720/":"https://svs.gsfc.nasa.gov/2915/"},
  {"moon_visualization_note",viewport_->preciseMoonFrameActive()?
   "JPL DE440 MOON_ME orientation and DE442s geometric Sun/Earth directions; no light-time/aberration correction":
   "Observed LROC global mosaic with fixed nonphysical studio lighting, no accurate lunar phase/libration"},
  {"moon_orientation",viewport_->preciseMoonFrameActive()?
   "JPL DE440 lunar binary PCK with matching MOON_ME FK":"unavailable"},
  {"earth_texture_kind","Observed NASA Blue Marble multi-date mosaic, NOT a live Earth image"},
  {"solar_geometry","Low-precision Sun illumination, NOT SPICE Earth orientation"},
  {"lunar_ephemeris",spice_.ready()?"JPL DE442s geometric J2000 states via NASA CSPICE":"unavailable"}
 };
 QSaveFile meta(outputPath+QStringLiteral(".provenance.json"));
 if(meta.open(QIODevice::WriteOnly)){
  const QByteArray data=QJsonDocument(provenance).toJson();
  if(meta.write(data)==data.size())meta.commit();else meta.cancelWriting();
 }
 return true;
}
bool MainWindow::captureViewport(const QString& outputPath){
 if(!viewport_->isValid()||!viewport_->hasObservedTexture())return false;
 const QImage pixels=viewport_->grabFramebuffer();
 if(pixels.isNull()||pixels.width()<100||pixels.height()<100||!pixels.save(outputPath,"PNG"))return false;
 QJsonObject provenance{
  {"application","Celestial Atlas 0.5.0"},
  {"capture_kind","Unmodified rendered OpenGL viewport screenshot"},
  {"simulated_utc",QDateTime::fromMSecsSinceEpoch(qint64(simulatedMilliseconds_),Qt::UTC).toString(Qt::ISODate)},
  {"earth_texture_source","https://svs.gsfc.nasa.gov/2915/"},
  {"rendered_body",viewport_->scene()==EarthViewport::Scene::Moon?"Moon":"Earth"},
  {"actual_render_texture",viewport_->scene()==EarthViewport::Scene::Moon?
   "https://svs.gsfc.nasa.gov/4720/":"https://svs.gsfc.nasa.gov/2915/"},
  {"moon_visualization_note",viewport_->preciseMoonFrameActive()?
   "JPL DE440 MOON_ME orientation and DE442s geometric Sun/Earth directions; no light-time/aberration correction":
   "Observed LROC global mosaic with fixed nonphysical studio lighting, no accurate lunar phase/libration"},
  {"moon_orientation",viewport_->preciseMoonFrameActive()?
   "JPL DE440 lunar binary PCK with matching MOON_ME FK":"unavailable"},
  {"earth_texture_kind","NASA observed multi-date satellite composite, NOT live imagery"},
  {"solar_geometry","Low-precision solar lighting, NOT SPICE Earth orientation"},
  {"lunar_ephemeris",spice_.ready()?"JPL DE442s geometric J2000 states via NASA CSPICE":"unavailable"}
 };
 QSaveFile meta(outputPath+QStringLiteral(".provenance.json"));
 if(meta.open(QIODevice::WriteOnly)){
  const QByteArray json=QJsonDocument(provenance).toJson();
  if(meta.write(json)==json.size())meta.commit();else meta.cancelWriting();
 }
 return true;
}
void MainWindow::setImageReady(const QString& path,const QString& hash){
 downloadButton_->setEnabled(true);
 downloadButton_->setText(QStringLiteral("更新 NASA 影像"));
 if(!viewport_->setTextureFile(path)){
  textureInfo_->setText(QStringLiteral("影像下载完成，但纹理加载失败"));return;
 }
 textureInfo_->setText(QStringLiteral("NASA Blue Marble 2048 × 1024\nSHA256: %1...\n已保存来源元数据").arg(hash.left(12)));
}
} // namespace atlas
