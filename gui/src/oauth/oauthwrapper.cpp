#include "oauthwrapper.h"

#include "githubwrapper.h"

#include <QtGui>
#include <QtCore>
#include <QtNetworkAuth>

const QUrl userdataUrl("https://api.github.com/user");//"https://gitlab.rlp.net/api/v4/user");

OAuthWrapper::OAuthWrapper(QObject *parent,QJsonObject configuration) : QObject(parent)
{
    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl(configuration.value("authurl").toString()));
    oauth2.setAccessTokenUrl(QUrl(configuration.value("tokenurl").toString()));
    oauth2.setScope(configuration.value("scope").toString());

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::Granted)
			qDebug() << "User authentication successfull." ;
            emit authenticated();
    });
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization && isPermanent())
            parameters->insert("duration", "permanent");
    });
    //connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
    //        &QDesktopServices::openUrl);
}

OAuthWrapper::OAuthWrapper(const QString &clientIdentifier, QObject *parent) :
    OAuthWrapper(parent)
{
    oauth2.setClientIdentifier(clientIdentifier);
}

bool OAuthWrapper::isPermanent() const
{
    return permanent;
}

void OAuthWrapper::setPermanent(bool value)
{
    permanent = value;
}

void OAuthWrapper::grant()
{
    oauth2.grant();
}

QNetworkReply * OAuthWrapper::requestUserData()
{
    qDebug() << "[OAuthWrapper:requestUserData]";
    return oauth2.get(QUrl(userdataUrl));
}

void OAuthWrapper::getUserData()
{
    qDebug() << "[OAuthWrapper:getUserData]";
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
