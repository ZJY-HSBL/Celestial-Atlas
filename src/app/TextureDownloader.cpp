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
TextureDownloader::TextureDownloader(QObject* parent):QObject(parent){}
QString TextureDownloader::sourceUrl(){
 return QStringLiteral("https://svs.gsfc.nasa.gov/vis/a000000/a002900/a002915/bluemarble-2048.png");
}
QString TextureDownloader::expectedImagePath(){
 return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/assets/nasa/bluemarble-2048.png");
}
void TextureDownloader::download(){
 if(downloading_)return;
 downloading_=true;
 QNetworkRequest req(QUrl(sourceUrl()));
 req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
 req.setRawHeader("User-Agent","CelestialAtlas/0.1.0 (educational astronomical viewer)");
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
 QImage image=QImage::fromData(bytes,"PNG");
 if(image.isNull()||image.width()!=2048||image.height()!=1024){
  emit failed(QStringLiteral("NASA 影像不是预期的 2048×1024 PNG"));return;
 }
 QString path=expectedImagePath();
 QFileInfo info(path);
 if(!QDir().mkpath(info.absolutePath())){emit failed(QStringLiteral("无法创建影像缓存目录"));return;}
 QSaveFile output(path);
 if(!output.open(QIODevice::WriteOnly)||output.write(bytes)!=bytes.size()||!output.commit()){
  emit failed(QStringLiteral("保存影像失败"));return;
 }
 QString hash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
 QJsonObject provenance{
  {"title","NASA Blue Marble: A Seamless Image Mosaic of the Earth"},
  {"source_page","https://svs.gsfc.nasa.gov/2915/"},
  {"source_url",sourceUrl()},
  {"credit","NASA/Goddard Space Flight Center Scientific Visualization Studio"},
  {"observation_note","Multi-date satellite composite, not live imagery"},
  {"projection","Equirectangular, longitude east positive"},
  {"sha256",hash}
 };
 QSaveFile meta(info.absolutePath()+"/bluemarble-2048.provenance.json");
 if(meta.open(QIODevice::WriteOnly)){
  QByteArray serialized=QJsonDocument(provenance).toJson();
  if(meta.write(serialized)==serialized.size())meta.commit();
  else meta.cancelWriting();
 }
 emit ready(path,hash);
}
}
