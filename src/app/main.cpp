#include "app/MainWindow.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <cmath>
#include "astronomy/SpiceEphemeris.hpp"
#include <QSurfaceFormat>
int main(int argc,char* argv[]){
 QSurfaceFormat f;
 f.setRenderableType(QSurfaceFormat::OpenGL);
 f.setVersion(3,3);
 f.setProfile(QSurfaceFormat::CoreProfile);
 f.setDepthBufferSize(24);
 f.setStencilBufferSize(8);
 f.setSamples(4);
 QSurfaceFormat::setDefaultFormat(f);
 QApplication app(argc,argv);
 QCoreApplication::setOrganizationName("CelestialAtlas");
 QCoreApplication::setApplicationName("CelestialAtlas");
 QCoreApplication::setApplicationVersion("0.7.0");
 QCommandLineParser parser;
 parser.setApplicationDescription("Celestial Atlas earth visualization and scientific capture");
 parser.addHelpOption();
 QCommandLineOption selfTestOpt("spice-self-test","Run actual NAIF SPICE lunar geometry and frame test using packaged kernels; exit with status.");
 parser.addOption(selfTestOpt);
 parser.addVersionOption();
 QCommandLineOption textureOpt({"t","texture"},"NASA Blue Marble PNG file path (2048x1024)","path");
 QCommandLineOption captureOpt({"s","screenshot"},"Capture actual OpenGL viewport PNG then exit","path");
 QCommandLineOption workbenchOpt({"w","workbench-screenshot"},"Capture entire actual Qt workspace PNG then exit","path");
 QCommandLineOption moonTextureOpt("moon-texture","NASA LROC 2048x1024 global texture JPEG path","path");
 QCommandLineOption sceneOpt("scene","Select earth, moon or system before capture","body");
 QCommandLineOption scaleOpt("system-scale","Earth-Moon distance setting: true or compressed","mode");
 parser.addOption(scaleOpt);
 QCommandLineOption utcOpt("utc","Set a fixed simulation UTC ISO 8601 time, such as 2024-03-25T07:00:00Z","timestamp");
 parser.addOption(utcOpt);
 parser.addOption(textureOpt);parser.addOption(moonTextureOpt);
 parser.addOption(sceneOpt);parser.addOption(captureOpt);parser.addOption(workbenchOpt);
 parser.process(app);
 if(parser.isSet(selfTestOpt)){
  const QString override=qEnvironmentVariable("ATLAS_SPICE_KERNEL_DIR");
  const QDir kernels(override.isEmpty()?
   QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("kernels")):override);
  atlas::astro::SpiceEphemeris spice;
  std::string error;
  if(!spice.loadKernels(kernels.filePath(QStringLiteral("naif0012.tls")).toStdString(),
                        kernels.filePath(QStringLiteral("de442s.bsp")).toStdString(),error)){
   std::cerr<<"NASA CSPICE LSK/SPK failed: "<<error<<'\n';return 10;
  }
  if(!spice.loadMoonOrientation(kernels.filePath(QStringLiteral("moon_de440_250416.tf")).toStdString(),
                                kernels.filePath(QStringLiteral("moon_pa_de440_200625.bpc")).toStdString(),error)){
   std::cerr<<"NASA lunar ME PCK/FK failed: "<<error<<'\n';return 11;
  }
  using namespace std::chrono;
  const auto time=sys_days{year_month_day{year{2024},month{4},day{8}}}+hours{18};
  atlas::astro::SolarLunarSample sample;
  atlas::astro::LunarBodyFrame body;
  if(!spice.sample(time,sample,error)||!spice.sampleMoonBodyFrame(time,body,error)){
   std::cerr<<"JPL state/orientation query failed: "<<error<<'\n';return 12;
  }
  if(!(sample.lunar.illuminatedFraction>=0&&sample.lunar.illuminatedFraction<0.01)||
     !(sample.lunar.moonDistanceKm>340000&&sample.lunar.moonDistanceKm<410000)||
     !std::isfinite(body.subEarthLongitudeDeg)){
   std::cerr<<"JPL data failed physical bounds check\n";return 13;
  }
  std::cout<<"NASA CSPICE VERIFIED: Moon distance="<<sample.lunar.moonDistanceKm
           <<" km, illumination="<<sample.lunar.illuminatedFraction
           <<", sub-Earth lon="<<body.subEarthLongitudeDeg<<" deg\n";
  return 0;
 }
 atlas::MainWindow window;
 if(parser.isSet(textureOpt) && !window.loadTextureFromPath(parser.value(textureOpt)))return 3;
 if(parser.isSet(moonTextureOpt) && !window.loadMoonTextureFromPath(parser.value(moonTextureOpt)))return 5;
 if(parser.value(sceneOpt).toLower()=="moon")window.selectMoonScene();
 else if(parser.value(sceneOpt).toLower()=="system")window.selectEarthMoonScene();
 if(parser.value(scaleOpt).toLower()=="true")window.setTrueEarthMoonScale(true);
 if(parser.isSet(utcOpt)){
  const QDateTime utc=QDateTime::fromString(parser.value(utcOpt),Qt::ISODate);
  if(!utc.isValid()||utc.timeSpec()==Qt::LocalTime)return 6;
  window.setSimulationTimeUtc(utc);
 }
 window.show();
 if(parser.isSet(captureOpt)||parser.isSet(workbenchOpt)){
  QTimer::singleShot(1200,&app,[&]{
   window.focusDaylight();
   QTimer::singleShot(1000,&app,[&]{
    bool ok=true;
    if(parser.isSet(captureOpt))ok=window.captureViewport(parser.value(captureOpt))&&ok;
    if(parser.isSet(workbenchOpt))ok=window.captureWorkbench(parser.value(workbenchOpt))&&ok;
    app.exit(ok?0:4);
   });
  });
 }
 return app.exec();
}
