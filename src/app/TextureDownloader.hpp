#pragma once
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
class QNetworkReply;
namespace atlas {
class TextureDownloader final : public QObject {
 Q_OBJECT
public:
 explicit TextureDownloader(QObject* parent=nullptr);
 static QString expectedImagePath();
 static QString sourceUrl();
 void download();
signals:
 void ready(const QString& localPath,const QString& sha256);
 void failed(const QString& message);
 void progress(qint64 received,qint64 total);
private:
 void finish(QNetworkReply* reply);
 QNetworkAccessManager network_;
 bool downloading_=false;
};
}
