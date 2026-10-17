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
 QCoreApplication::setApplicationVersion("0.2.0");
 QCommandLineParser parser;
 parser.setApplicationDescription("Celestial Atlas earth visualization and scientific capture");
 parser.addHelpOption();
 parser.addVersionOption();
 QCommandLineOption textureOpt({"t","texture"},"NASA Blue Marble PNG file path (2048x1024)","path");
 QCommandLineOption captureOpt({"s","screenshot"},"Capture actual OpenGL viewport PNG then exit","path");
 parser.addOption(textureOpt);parser.addOption(captureOpt);
 parser.process(app);
 atlas::MainWindow window;
 if(parser.isSet(textureOpt) && !window.loadTextureFromPath(parser.value(textureOpt)))return 3;
 window.show();
 if(parser.isSet(captureOpt)){
  QTimer::singleShot(2500,&app,[&]{
   app.exit(window.captureViewport(parser.value(captureOpt))?0:4);
  });
 }
 return app.exec();
}
