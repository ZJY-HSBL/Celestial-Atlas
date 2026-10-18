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
MainWindow::MainWindow(QWidget* parent):QMainWindow(parent),downloader_(this){
 setWindowTitle(QStringLiteral("Celestial Atlas | 科学宇宙探索"));
 resize(1240,790);setMinimumSize(860,540);
 auto* page=new QWidget(this);
 auto* root=new QVBoxLayout(page);
 root->setContentsMargins(14,14,14,14);root->setSpacing(12);
 auto* header=new QHBoxLayout;
 header->addWidget(label(QStringLiteral("✦  CELESTIAL ATLAS    /    地球科学观测"),"heading"));
 header->addStretch();
 downloadButton_=new QPushButton(QStringLiteral("下载 NASA 真实影像"));
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
 leftLayout->addWidget(label(QStringLiteral("●  地球空间视图"),"activeItem"));
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
 rightLayout->addWidget(label(QStringLiteral("地球  /  EARTH"),"title"));
 rightLayout->addWidget(label(QStringLiteral("太阳系 · 第三颗行星"),"detail"));
 rightLayout->addSpacing(12);
 rightLayout->addWidget(label(QStringLiteral("平均半径    6,371 km"),"data"));
 rightLayout->addWidget(label(QStringLiteral("太阳光照与地球旋转"),"section"));
 solarInfo_=label(QStringLiteral("正在计算..."),"data");
 rightLayout->addWidget(solarInfo_);
 rightLayout->addWidget(label(QStringLiteral("MOON / 地月系统"),"section"));
 moonInfo_=label(QStringLiteral("精密月相尚未加载"),"data");
 rightLayout->addWidget(moonInfo_);
 auto* moonPhoto=new QPushButton(QStringLiteral("NASA LRO 月球真实观测影像"));
 connect(moonPhoto,&QPushButton::clicked,this,[]{
  QDesktopServices::openUrl(QUrl(QStringLiteral("https://science.nasa.gov/resource/moon-mosaic/")));
 });
 rightLayout->addWidget(moonPhoto);
 rightLayout->addStretch();
 rightLayout->addWidget(label(QStringLiteral(
  "SCIENCE DISCLOSURE\n"
  "太阳方向：低精度近似算法（尚非 JPL 精密星历）\n"
  "地球影像：NASA 多日期卫星拼接，非实时云图\n"
  "大气光照：可视化近似，不是辐射传输模型"),"caution"));
 auto* focus=new QPushButton(QStringLiteral("聚焦当前地球白昼面"));
 connect(focus,&QPushButton::clicked,viewport_,&EarthViewport::focusDaylight);
 rightLayout->addWidget(focus);
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
 workspace->addWidget(right);
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
 speed->addItem(QStringLiteral("暂停"),0.0);
 speed->addItem(QStringLiteral("1 × 真实时间"),1.0);
 speed->addItem(QStringLiteral("60 ×"),60.0);
 speed->addItem(QStringLiteral("3600 ×"),3600.0);
 speed->setCurrentIndex(1);footer->addWidget(speed);
 footer->addStretch();
 status_=label(QStringLiteral("本地科学演示 V0.3.0"),"detail");
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
 connect(downloadButton_,&QPushButton::clicked,&downloader_,&TextureDownloader::download);
 connect(&downloader_,&TextureDownloader::ready,this,&MainWindow::setImageReady);
 connect(&downloader_,&TextureDownloader::failed,this,[this](const QString& error){
  downloadButton_->setEnabled(true);
  downloadButton_->setText(QStringLiteral("重试 NASA 影像下载"));
  textureInfo_->setText(error);
 });
 connect(&downloader_,&TextureDownloader::progress,this,[this](qint64 received,qint64 total){
  if(total>0)downloadButton_->setText(QStringLiteral("影像下载 %1%").arg(100*received/total));
 });
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
 }
}
void MainWindow::focusDaylight(){viewport_->focusDaylight();}
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
  {"application","Celestial Atlas 0.3.0"},
  {"capture_kind","Full actual Qt application window including native OpenGL viewport"},
  {"simulated_utc",QDateTime::fromMSecsSinceEpoch(qint64(simulatedMilliseconds_),Qt::UTC).toString(Qt::ISODate)},
  {"earth_texture_source","https://svs.gsfc.nasa.gov/2915/"},
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
  {"application","Celestial Atlas 0.3.0"},
  {"capture_kind","Unmodified rendered OpenGL viewport screenshot"},
  {"simulated_utc",QDateTime::fromMSecsSinceEpoch(qint64(simulatedMilliseconds_),Qt::UTC).toString(Qt::ISODate)},
  {"earth_texture_source","https://svs.gsfc.nasa.gov/2915/"},
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
