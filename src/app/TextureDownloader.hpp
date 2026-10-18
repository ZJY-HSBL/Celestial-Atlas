#pragma once
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
class QNetworkReply;
namespace atlas {
class TextureDownloader final : public QObject {
 Q_OBJECT
public:
 enum class Body { Earth, Moon };
 explicit TextureDownloader(QObject* parent=nullptr,Body body=Body::Earth);
 static QString expectedImagePath();
 static QString sourceUrl();
 static QString moonSourceUrl();
 static QString moonExpectedImagePath();
 void download();
signals:
 void ready(const QString& localPath,const QString& sha256);
 void failed(const QString& message);
 void progress(qint64 received,qint64 total);
private:
 void finish(QNetworkReply* reply);
 QNetworkAccessManager network_;
 bool downloading_=false;
 Body body_=Body::Earth;
};
}
