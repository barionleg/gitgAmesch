//
// Created by timo on 2/12/23.
//

#include <QtGui>
#include <QDialog>
#include <QGridLayout>
#include <QListWidget>

#include "QGMMacros.h"

#ifndef GIGAMESH_QGMANNOTATIONDIALOG_H
#define GIGAMESH_QGMANNOTATIONDIALOG_H

class QGMAnnotationDialog : public QDialog {
    Q_OBJECT

    QTabWidget* tabw;

public:
    //QGMAnnotationDialog(QJsonObject templatejson,QWidget *parent = nullptr);
    QGMAnnotationDialog(QJsonObject templatejson, QJsonObject annodata,QWidget *parent = nullptr);
    void createInputFieldByType(const QString& inputtype,int linecounter,const QString& key,QGridLayout* gridLayout,QWidget* curwidget,const QJsonObject& data,bool hasdata,QJsonArray curanno);
    void addCategoryIndependentFields(QWidget* curwidget,int linecounter,QGridLayout* gridLayout,QJsonArray curanno,QJsonArray tags);
    signals:

public slots:

    // QWidget interface
protected:

    QJsonObject saveAnnotationJSON();
};

#endif //GIGAMESH_QGMANNOTATIONDIALOG_H
