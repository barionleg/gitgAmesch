#include "githubmodel.h"

GithubModel::GithubModel(QObject *parent) : QAbstractTableModel(parent) {}

GithubModel::GithubModel(const QString &clientId, QObject *parent) :
    QAbstractTableModel(parent),
    githubWrapper(clientId)
{
    qDebug() << "[GithubModel::githubmodel()]";
    grant();
}

int GithubModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return threads.size();
}

int GithubModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return threads.size() ? 1 : 0;
}

QVariant GithubModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto childrenObject = threads.at(index.row());
        Q_ASSERT(childrenObject.value("data").isObject());
        const auto dataObject = childrenObject.value("data").toObject();
        return dataObject.value("title").toString();
    }
    return QVariant();
}


void GithubModel::grant()
{
    qDebug() << "[GithubModel::grant()]";
    githubWrapper.grant();
    connect(&githubWrapper, &GithubWrapper::authenticated, this, &GithubModel::update);
    emit update();
}

void GithubModel::update()
{
    qDebug() << "[GithubModel:update]";

    auto reply = githubWrapper.requestUserData();
    githubWrapper.getUserData();

    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            return;
        }
        qDebug() << "[GithubModel:update] NetworkReply finished";

        const auto json = reply->readAll();
        const auto document = QJsonDocument::fromJson(json);
        Q_ASSERT(document.isObject());
        const auto rootObject = document.object();
        if(!document.isEmpty()){
            qDebug() << "[GithubModel:update] JsonDocument is empty.";
        }
        if(document.isArray()){
            qDebug() << "[GithubModel:update] Json Document contains Array.";
        }
        Q_ASSERT(rootObject.value("kind").toString() == "Listing");
        const auto dataValue = rootObject.value("data");
        Q_ASSERT(dataValue.isObject());
        const auto dataObject = dataValue.toObject();
        const auto childrenValue = dataObject.value("children");
        Q_ASSERT(childrenValue.isArray());
        const auto childrenArray = childrenValue.toArray();

        if (childrenArray.isEmpty())
            return;

        beginInsertRows(QModelIndex(), threads.size(), childrenArray.size() + threads.size() - 1);
        for (const auto childValue : qAsConst(childrenArray)) {
            Q_ASSERT(childValue.isObject());
            threads.append(childValue.toObject());
        }
        endInsertRows();
    });
}
