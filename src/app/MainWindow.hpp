#pragma once
#include "app/TextureDownloader.hpp"
#include "astronomy/SpiceEphemeris.hpp"
#include <QDateTimeEdit>
#include <QComboBox>
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
 bool loadMoonTextureFromPath(const QString& path);
 void selectMoonScene();
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
 QLabel* selectedName_=nullptr;
 QLabel* selectedRadius_=nullptr;
 QLabel* headerTitle_=nullptr;
 QLabel* earthNav_=nullptr;
 QLabel* moonNav_=nullptr;
 QLabel* solarHeader_=nullptr;
 QLabel* selectedSubtitle_=nullptr;
 QComboBox* sceneSelector_=nullptr;
 QLabel* lunarMapStatus_=nullptr;
 QLabel* moonInfo_=nullptr;
 astro::SpiceEphemeris spice_;
 QLabel* textureInfo_=nullptr;
 QLabel* status_=nullptr;
 QPushButton* downloadButton_=nullptr;
 TextureDownloader downloader_;
 TextureDownloader moonDownloader_;
 QTimer frameTimer_;
 QElapsedTimer elapsed_;
 double simulatedMilliseconds_=0.0;
 double simulationSpeed_=1.0;
 int frames_=0;
};
}
