#include "app/MainWindow.hpp"
#include <QApplication>
#include <QCoreApplication>
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
 QCoreApplication::setApplicationVersion("0.1.0");
 atlas::MainWindow window;
 window.show();
 return app.exec();
}
