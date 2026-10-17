#pragma once
#include "app/TextureDownloader.hpp"
#include <QDateTimeEdit>
#include <QElapsedTimer>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
namespace atlas {
class EarthViewport;
class MainWindow final:public QMainWindow {
 Q_OBJECT
public:
 explicit MainWindow(QWidget* parent=nullptr);
 bool loadTextureFromPath(const QString& path);
 void focusDaylight();
 bool captureViewport(const QString& outputPath);
 bool captureWorkbench(const QString& outputPath);
private:
 void tick();
 void refreshStatus();
 void setImageReady(const QString& path,const QString& hash);
 EarthViewport* viewport_=nullptr;
 QDateTimeEdit* timeEdit_=nullptr;
 QLabel* solarInfo_=nullptr;
 QLabel* textureInfo_=nullptr;
 QLabel* status_=nullptr;
 QPushButton* downloadButton_=nullptr;
 TextureDownloader downloader_;
 QTimer frameTimer_;
 QElapsedTimer elapsed_;
 double simulatedMilliseconds_=0.0;
 double simulationSpeed_=1.0;
 int frames_=0;
};
}
