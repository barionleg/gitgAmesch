#ifndef GITHUBWRAPPER_H
#define GITHUBWRAPPER_H

#include <QtCore>
#include <QtNetwork>
#include <QOAuth2AuthorizationCodeFlow>

#include "oauthwrapper.h"

class GithubWrapper : public OAuthWrapper
{

public:
    GithubWrapper(QObject *parent = nullptr);
    GithubWrapper(const QString &clientIdentifier, QObject *parent = nullptr);

public slots:
    void getUserData();

};


#endif // GITHUBWRAPPER_H
