#ifndef OAUTHWRAPPER_H
#define OAUTHWRAPPER_H

#include <QtCore>
#include <QtNetwork>

#include <QOAuth2AuthorizationCodeFlow>

class OAuthWrapper : public QObject
{
    Q_OBJECT

public:
    OAuthWrapper(QObject *parent = nullptr,QJsonObject configuration=QJsonObject());
    OAuthWrapper(const QString &clientIdentifier, QObject *parent = nullptr);

    bool isPermanent() const;
    void setPermanent(bool value);

    QNetworkReply * requestUserData();

public slots:
    void grant();
    virtual void getUserData();

signals:
    void authenticated();
    void subscribed(const QUrl &url);

protected:
    QOAuth2AuthorizationCodeFlow oauth2;
    bool permanent = false;
};
#endif // OAUTHWRAPPER_H
