#include "githubwrapper.h"

#include <QtGui>
#include <QtCore>
#include <QtNetworkAuth>
#include <iostream>

const QUrl userdataUrl("https://api.github.com/users");//"https://gitlab.rlp.net/api/v4/user");

GithubWrapper::GithubWrapper(QObject *parent) : OAuthWrapper(parent)
{
    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl("http://github.com/login/oauth/authorize"));
    oauth2.setAccessTokenUrl(QUrl("https://github.com/login/oauth/access_token"));
    oauth2.setScope("read:user"); //"api");

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
			qDebug() << "User authentication." ;
        if (status == QAbstractOAuth::Status::Granted)
			
            emit authenticated();
    });
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization && isPermanent())
            parameters->insert("duration", "permanent");
    });
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
}

GithubWrapper::GithubWrapper(const QString &clientIdentifier, QObject *parent) :
    GithubWrapper(parent)
{
    oauth2.setClientIdentifier(clientIdentifier);
	std::cout << oauth2.clientIdentifier().toStdString();
}

void GithubWrapper::getUserData()
{
    qDebug() << "Susbscribing...";
    QNetworkReply *reply = oauth2.get(userdataUrl);
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << "Reddit error:" << reply->errorString();
            return;
        }

        const auto json = reply->readAll();

        const auto document = QJsonDocument::fromJson(json);
        qDebug() << document;
        Q_ASSERT(document.isObject());
        const auto rootObject = document.object();
        const auto dataValue = rootObject.value("data");
        Q_ASSERT(dataValue.isObject());
        const auto dataObject = dataValue.toObject();
        const auto websocketUrlValue = dataObject.value("websocket_url");
        Q_ASSERT(websocketUrlValue.isString() && websocketUrlValue.toString().size());
        const QUrl websocketUrl(websocketUrlValue.toString());
        emit subscribed(websocketUrl);
    });
}
