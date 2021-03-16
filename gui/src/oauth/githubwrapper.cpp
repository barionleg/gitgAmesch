#include "githubwrapper.h"

#include <QtGui>
#include <QtCore>
#include <QtNetworkAuth>
#include <iostream>

const QUrl userdataUrl("https://api.github.com/user");//"https://gitlab.rlp.net/api/v4/user");

GithubWrapper::GithubWrapper(QObject *parent) : OAuthWrapper(parent)
{
    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl("http://github.com/login/oauth/authorize"));
    oauth2.setAccessTokenUrl(QUrl("https://github.com/login/oauth/access_token"));
    //oauth2.setScope("read:user"); //"api");

    qDebug() << "[githubwrapper:callback] " << replyHandler->callback();
    qDebug() << "[githubwrapper:token] " << oauth2.token();

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
        qDebug() << "[GithubWrapper::authenticating]" ;
        if (status == QAbstractOAuth::Status::Granted)
            emit authenticated();
    });
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization && isPermanent())
            parameters->insert("duration", "permanent");
        // parameters->insert("redirect_uri","http://localhost:1337");
        parameters->insert("login","feedelamort");
    });
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);

    switch(oauth2.status())
    {
        case QAbstractOAuth::Status::NotAuthenticated : qDebug() << "Not Authenticated" ;
        case QAbstractOAuth::Status::TemporaryCredentialsReceived : qDebug() << "Temporary Credentials Received" ;
        case QAbstractOAuth::Status::Granted : qDebug() << "Granted" ;
        case QAbstractOAuth::Status::RefreshingToken : qDebug() << "Refreshing" ;
    }

    QByteArray data;
    //connect(&replyHandler, QAbstractOAuthReplyHandler::callbackDataReceived(data),
    //        this, GithubWrapper::receiveReply)

    qDebug() << "[githubwrapper:token] " << oauth2.token();

}

void GithubWrapper::receiveReply()
{
    qDebug() << "[githubwrapper:replyReceived] ";
    return;
}

GithubWrapper::GithubWrapper(const QString &clientIdentifier, QObject *parent) :
    GithubWrapper(parent)
{
    qDebug() << "[GithubWrapper:githubrapper()]";
    oauth2.setClientIdentifier(clientIdentifier);
}

void GithubWrapper::getUserData()
{
    qDebug() << "[GithubWrapper:getUserData]";

    QNetworkReply *reply = oauth2.get(userdataUrl);

    qDebug() << "[GithubWrapper] User Data Url = " << reply->url().toDisplayString(QUrl::FullyDecoded);

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Github error:" << reply->errorString();
        return;
    }

    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << "Github error:" << reply->errorString();
            return;
        }

        qDebug() << "[GithubModel:update] NetworkReply finished";

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
