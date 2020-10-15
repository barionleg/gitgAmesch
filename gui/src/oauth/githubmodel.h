#ifndef GITHUBMODEL_H
#define GITHUBMODEL_H

#include <QtCore>

#include "githubwrapper.h"

QT_FORWARD_DECLARE_CLASS(QNetworkReply)

class GithubModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    GithubModel(QObject *parent = nullptr);
    GithubModel(const QString &clientId, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void grant();

signals:
    void error(const QString &errorString);

private slots:
    void update();

private:
    GithubWrapper githubWrapper;
    QPointer<QNetworkReply> liveThreadReply;
    QList<QJsonObject> threads;
};


#endif // GITHUBMODEL_H
