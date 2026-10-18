#include "app/TextureDownloader.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUrl>
namespace atlas {
TextureDownloader::TextureDownloader(QObject* parent,Body body):QObject(parent),body_(body){}
QString TextureDownloader::sourceUrl(){
 return QStringLiteral("https://svs.gsfc.nasa.gov/vis/a000000/a002900/a002915/bluemarble-2048.png");
}
QString TextureDownloader::moonSourceUrl(){
 return QStringLiteral("https://svs.gsfc.nasa.gov/vis/a000000/a004700/a004720/lroc_color_2k.jpg");
}
QString TextureDownloader::moonExpectedImagePath(){
 return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/assets/nasa/lroc_color_2k.jpg");
}
QString TextureDownloader::expectedImagePath(){
 return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/assets/nasa/bluemarble-2048.png");
}
void TextureDownloader::download(){
 if(downloading_)return;
 downloading_=true;
 QNetworkRequest req{QUrl(body_==Body::Earth?sourceUrl():moonSourceUrl())};
 req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
 req.setRawHeader("User-Agent","CelestialAtlas/0.4.0 (NASA observed maps)");
 QNetworkReply* reply=network_.get(req);
 connect(reply,&QNetworkReply::downloadProgress,this,&TextureDownloader::progress);
 connect(reply,&QNetworkReply::finished,this,[this,reply]{
  finish(reply);reply->deleteLater();downloading_=false;
 });
}
void TextureDownloader::finish(QNetworkReply* reply){
 if(reply->error()!=QNetworkReply::NoError){
  emit failed(QStringLiteral("NASA 影像下载失败：")+reply->errorString());return;
 }
 int http=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
 if(http!=200){emit failed(QStringLiteral("NASA 影像 HTTP 状态：%1").arg(http));return;}
 QByteArray bytes=reply->readAll();
 if(bytes.isEmpty()||bytes.size()>16*1024*1024){emit failed(QStringLiteral("NASA 影像大小不符合预期"));return;}
 QImage image=QImage::fromData(bytes,body_==Body::Earth?"PNG":"JPG");
 if(image.isNull()||image.width()!=2048||image.height()!=1024){
  emit failed(QStringLiteral("NASA 影像不是预期的 2048×1024 影像格式"));return;
 }
 QString path=body_==Body::Earth?expectedImagePath():moonExpectedImagePath();
 QFileInfo info(path);
 if(!QDir().mkpath(info.absolutePath())){emit failed(QStringLiteral("无法创建影像缓存目录"));return;}
 QSaveFile output(path);
 if(!output.open(QIODevice::WriteOnly)||output.write(bytes)!=bytes.size()||!output.commit()){
  emit failed(QStringLiteral("保存影像失败"));return;
 }
 QString hash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
 QJsonObject provenance{
  {"title",body_==Body::Earth?"NASA Blue Marble":"NASA LROC CGI Moon Kit 2025 2K global color map"},
  {"source_page",body_==Body::Earth?"https://svs.gsfc.nasa.gov/2915/":"https://svs.gsfc.nasa.gov/4720/"},
  {"source_url",body_==Body::Earth?sourceUrl():moonSourceUrl()},
  {"credit","NASA/Goddard Space Flight Center Scientific Visualization Studio; NASA LRO data"},
  {"observation_note",body_==Body::Earth?
    "Observed multi-date Earth satellite composite, not live":
    "LROC WAC observed mosaic processed for visualization; polar regions supplemented by LOLA albedo; NOT a time-resolved or raw science image"},
  {"projection","global equirectangular map, center longitude 0"},
  {"sha256",hash}
 };
 const QString metadata=info.absolutePath()+
   (body_==Body::Earth?QStringLiteral("/bluemarble-2048.provenance.json"):
                      QStringLiteral("/lroc_color_2k.provenance.json"));
 QSaveFile meta(metadata);
 if(meta.open(QIODevice::WriteOnly)){
  QByteArray serialized=QJsonDocument(provenance).toJson();
  if(meta.write(serialized)==serialized.size())meta.commit();
  else meta.cancelWriting();
 }
 emit ready(path,hash);
}
}
