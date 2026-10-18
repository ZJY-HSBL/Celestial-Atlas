#include "app/MainWindow.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
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
 QCoreApplication::setApplicationVersion("0.4.1");
 QCommandLineParser parser;
 parser.setApplicationDescription("Celestial Atlas earth visualization and scientific capture");
 parser.addHelpOption();
 parser.addVersionOption();
 QCommandLineOption textureOpt({"t","texture"},"NASA Blue Marble PNG file path (2048x1024)","path");
 QCommandLineOption captureOpt({"s","screenshot"},"Capture actual OpenGL viewport PNG then exit","path");
 QCommandLineOption workbenchOpt({"w","workbench-screenshot"},"Capture entire actual Qt workspace PNG then exit","path");
 QCommandLineOption moonTextureOpt("moon-texture","NASA LROC 2048x1024 global texture JPEG path","path");
 QCommandLineOption sceneOpt("scene","Select 'earth' or 'moon' before capture","body");
 parser.addOption(textureOpt);parser.addOption(moonTextureOpt);
 parser.addOption(sceneOpt);parser.addOption(captureOpt);parser.addOption(workbenchOpt);
 parser.process(app);
 atlas::MainWindow window;
 if(parser.isSet(textureOpt) && !window.loadTextureFromPath(parser.value(textureOpt)))return 3;
 if(parser.isSet(moonTextureOpt) && !window.loadMoonTextureFromPath(parser.value(moonTextureOpt)))return 5;
 if(parser.value(sceneOpt).toLower()=="moon")window.selectMoonScene();
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
