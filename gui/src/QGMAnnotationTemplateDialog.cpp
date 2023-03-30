//
// Created by timo on 2/12/23.
//
#include <QUrl>
#include <QtGui>
#include <QTableWidgetItem>

#include "QGMAnnotationTemplateDialog.h"


QGMAnnotationTemplateDialog::QGMAnnotationTemplateDialog(QString templatepath,QWidget *parent){
    QFile jsonfile;
    jsonfile.setFileName(templatepath);
    jsonfile.open(QIODevice::ReadOnly);
    QByteArray data = jsonfile.readAll();
    qDebug()<<QJsonDocument::fromJson(data);
    QJsonDocument annoDoc;
    annoDoc=QJsonDocument::fromJson(data);
    this->templatejson=annoDoc.array();
    setupUi( this );
    for(auto && i : templatejson){
        this->templateCBox->addItem(i.toObject().value("name").toString());
    }
    connect(this->templateCBox,SIGNAL(currentIndexChanged(const QString&)),
            this,SLOT(reloadTable));
    connect(this->newRowButton,SIGNAL(currentIndexChanged(const QString&)),
            this,SLOT(addRow));
    this->reloadTable();
    setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
    this->show();
}

void QGMAnnotationTemplateDialog::addRow(){
    this->annotemplateWidget->insertRow ( this->annotemplateWidget->rowCount());
}

void QGMAnnotationTemplateDialog::reloadTable(){
    int currentIndex=this->templateCBox->currentIndex();
    QJsonObject curtemplate=this->templatejson.at(currentIndex).toObject();
    this->annotemplateWidget->setRowCount(0);
    this->annotagWidget->setRowCount(0);
    for(const QString& key:curtemplate.value("fields").toObject().keys()){
        QJsonObject curobj=curtemplate.value("fields").toObject().value(key).toObject();
        this->annotemplateWidget->insertRow ( this->annotemplateWidget->rowCount());
        this->annotemplateWidget->setItem   ( this->annotemplateWidget->rowCount()-1,
                                 0,
                                 new QTableWidgetItem(key));
        this->annotemplateWidget->setItem   ( this->annotemplateWidget->rowCount()-1,
                                              1,
                                              new QTableWidgetItem(curobj.value("inputtype").toString()));
        this->annotemplateWidget->setItem   ( this->annotemplateWidget->rowCount()-1,
                                              2,
                                              new QTableWidgetItem(curobj.value("uri").toString()));
        if(curobj.contains("data")){
            QString datalist="[";
            for(const QString& dataitem:curobj.value("data").toObject().keys()){
                datalist=datalist+dataitem+" ";
            }
            datalist+="]";
            this->annotemplateWidget->setItem   ( this->annotemplateWidget->rowCount()-1,
                                                  3,
                                                  new QTableWidgetItem(datalist));
        }
    }
    if(curtemplate.contains("tags")) {
        int i=0;
        for(i=0;i<curtemplate.value("tags").toArray().size();i++){
            QJsonObject curobj = curtemplate.value("tags").toArray().at(i).toObject();
            this->annotagWidget->insertRow(this->annotagWidget->rowCount());
            this->annotagWidget->setItem   ( this->annotagWidget->rowCount()-1,
                                             0,
                                             new QTableWidgetItem(curobj.value("label").toString()));
            this->annotagWidget->setItem   ( this->annotagWidget->rowCount()-1,
                                             1,
                                             new QTableWidgetItem(curobj.value("uri").toString()));
        }
    }
}

void QGMAnnotationTemplateDialog::saveAsJSON(QString templatename) {
    for (int i = 0; i < templatejson.count(); i++) {
        QJsonObject curobj = templatejson.at(i).toObject();
        if (curobj.contains("name") && curobj.value("name").toString() == templatename) {
            curobj.remove("tags");
            curobj.insert("tags",QJsonArray());
            const QJsonArray &thetags = curobj.value("tags").toArray();
            for (int j = 0; j < this->annotagWidget->rowCount(); j++) {
                thetags.operator+(QJsonObject());
                thetags.at(j).toObject()["tag"]=this->annotagWidget->item(j, 0)->text();
                thetags.at(j).toObject()["uri"]=this->annotagWidget->item(j, 1)->text();
            }
            const QJsonObject &thefields = curobj.value("fields").toObject();
            for (int j = 0; j < this->annotemplateWidget->rowCount(); j++) {
                thefields[this->annotemplateWidget->item(j,0)->text()]=QJsonObject();
                QJsonObject curobject=thefields.value(this->annotemplateWidget->item(j,0)->text()).toObject();
                curobject.insert(QString("inputtype"),this->annotemplateWidget->item(j,1)->text());
                curobject.insert(QString("uri"),this->annotemplateWidget->item(j,2)->text());
                curobject.insert(QString("category"),this->annotemplateWidget->item(j,3)->text());
                curobject.insert(QString("datalist"),this->annotemplateWidget->item(j,4)->text());
            }
        }

    }
}

QGMAnnotationTemplateDialog::QGMAnnotationTemplateDialog(QJsonArray templatejson,QWidget *parent){
    //! Constructor
    setupUi( this );
    for(auto && i : templatejson){
        this->templateCBox->addItem(i.toObject().value("name").toString());
    }
    setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
    this->show();
}