//
// Created by timo on 2/12/23.
//

#include "QGMAnnotationDialog.h"
#include "CompleterDelegate.h"
#include <iostream>
#include <ostream>
#include <QUrl>
#include <QFileDialog>
#include <QtSvg/QSvgWidget>
#include <QVBoxLayout>
#include <QtWidgets>
#include <QDialog>


QGMAnnotationDialog::QGMAnnotationDialog(QJsonObject annotemplate,QJsonObject annodata,QWidget *parent) : QDialog( parent) {
    auto * gridLayout = new QGridLayout(this);
    int linecounter=0;
    auto tablabels=new QHash<QString,int>();
    auto * titlelabel=new QLabel(this);
    QString firstkey=annodata.keys().at(0);
    QJsonArray curanno=annodata.value(firstkey).toObject().value("body").toArray();
    titlelabel->setText("Edit Annotation "+firstkey);
    gridLayout->addWidget(titlelabel,0,0,1,2);
    tabw=new QTabWidget(this);
    QJsonArray thetags=annotemplate.value("tags").toArray();
    gridLayout->addWidget(tabw,1,0,1,2);
    if(annotemplate.contains("fields")){
        for(const QString& key:annotemplate.value("fields").toObject().keys()) {
            QJsonObject curobj = annotemplate.value("fields").toObject().value(key).toObject();
            if (curobj.contains("category")) {
                const QJsonArray &cats = curobj.value("category").toArray();
                for (const auto & cat : cats) {
                    QString curcat = cat.toString();
                    if (!(tablabels->contains(curcat))) {
                        auto *curwidget = new QWidget();
                        curwidget->setLayout(new QGridLayout());
                        tabw->addTab(curwidget, curcat);
                        tablabels->insert(curcat, tabw->count() - 1);
                    }
                }
            }
        }
        if(tabw->count()==0) {
            auto *curwidget = new QWidget();
            tabw->addTab(curwidget, "Annotation");
        }
        for(const QString& key:annotemplate.value("fields").toObject().keys()) {
            QJsonObject curobj = annotemplate.value("fields").toObject().value(key).toObject();
            QString inputtype=curobj.value("inputtype").toString();
            if (curobj.contains("category")) {
                const QJsonArray &cats = curobj.value("category").toArray();
                for (const auto & cat : cats) {
                    QWidget *curwidget = tabw->widget(tablabels->value(cat.toString()));
                    createInputFieldByType(inputtype, linecounter, key,
                                           dynamic_cast<QGridLayout *>(curwidget->layout()), curwidget, curobj.value("data").toObject(), curobj.contains("data"),curanno);
                }
            }else{
                QWidget *curwidget = tabw->widget(0);
                createInputFieldByType(inputtype, linecounter, key, dynamic_cast<QGridLayout *>(curwidget->layout()), curwidget, curobj.value("data").toObject(), curobj.contains("data"),curanno);
            }
            linecounter++;
        }
        for(int i=0;i<tabw->count();i++){
            QWidget *curwidget = tabw->widget(i);
            auto* gridl= dynamic_cast<QGridLayout *>(curwidget->layout());
            addCategoryIndependentFields(curwidget,gridl->rowCount(),gridl,curanno,thetags);
        }
        linecounter++;
    }
    auto * okbutton=new QPushButton(this);
    okbutton->setText("Ok");
    gridLayout->addWidget(okbutton,linecounter,1);
    auto * cancelbutton=new QPushButton(this);
    cancelbutton->setText("Cancel");
    gridLayout->addWidget(cancelbutton,linecounter,0);
    connect(okbutton,SIGNAL(clicked),
            this,SLOT(this->close));
    connect(cancelbutton,SIGNAL(clicked),
            this,SLOT(this->close));
    this->setLayout(gridLayout);
    setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
    this->show();
}

QStringList getTagsFromAnnotation(const QJsonArray& curanno){
    QStringList result;
    for(auto && i : curanno){
        QJsonObject curobj=i.toObject();
        if(curobj.contains("purpose") && curobj.value("purpose")=="tagging"){
            result.append(curobj.value("value").toString());
        }
    }
    return result;
}

QHash<QString,QString> getValuesFromAnnotation(const QJsonArray& curanno){
    QHash<QString,QString> result;
    for(auto && i : curanno){
        QJsonObject curobj=i.toObject();
        if(curobj.contains("purpose") && curobj.value("purpose")!="tagging"){
            result.insert(curobj.value("purpose").toString(),curobj.value("value").toString());
        }
    }
    return result;
}

void addTag(){

}

void QGMAnnotationDialog::addCategoryIndependentFields(QWidget* curwidget,int linecounter,QGridLayout* gridLayout,QJsonArray curanno,QJsonArray tags){
    auto * label = new QLabel(curwidget);
    label->setText("Comment");
    auto * pedit=new QPlainTextEdit(curwidget);
    gridLayout->addWidget(label,linecounter,0);
    gridLayout->addWidget(pedit,linecounter,1);
    linecounter++;
    auto * label2 = new QLabel(curwidget);
    label2->setText("Tags:");
    auto * ledit=new QListWidget(curwidget);
    QStringList assignedTags=getTagsFromAnnotation(curanno);
    if(!assignedTags.isEmpty()){
        for(const QString& tag:assignedTags){
            auto *it = new QListWidgetItem(tag);
            it->setFlags(it->flags()| Qt::ItemIsEditable);
            ledit->addItem(it);
            auto *delegate = new CompleterDelegate(curwidget,tags);
            ledit->setItemDelegate(delegate);
            ledit->setEditTriggers(QAbstractItemView::AllEditTriggers);
        }
    }else{
        auto *it = new QListWidgetItem("");
        it->setFlags(it->flags()| Qt::ItemIsEditable);
        ledit->addItem(it);
        auto *delegate = new CompleterDelegate(curwidget,tags);
        ledit->setItemDelegate(delegate);
        ledit->setEditTriggers(QAbstractItemView::AllEditTriggers);
    }
    auto * addTag=new QPushButton(curwidget);
    addTag->setText("Add Tag");
    connect(addTag, SIGNAL(clicked()), this, SLOT(addTag));
    gridLayout->addWidget(label2,linecounter,0);
    gridLayout->addWidget(ledit,linecounter,1);
    gridLayout->addWidget(addTag,linecounter,2);
    linecounter++;
    auto * semtaglabel = new QLabel(curwidget);
    semtaglabel->setText("Semantic Tags:");
    auto * semtagedit=new QListWidget(curwidget);
    auto *it = new QListWidgetItem("");
    it->setFlags(it->flags()| Qt::ItemIsEditable);
    semtagedit->addItem(it);
    auto *delegate = new CompleterDelegate(curwidget,tags);
    semtagedit->setItemDelegate(delegate);
    semtagedit->setEditTriggers(QAbstractItemView::AllEditTriggers);
    auto * addSemTag=new QPushButton(curwidget);
    addSemTag->setText("Add Semantic Tag");
    gridLayout->addWidget(semtaglabel,linecounter,0);
    gridLayout->addWidget(semtagedit,linecounter,1);
    gridLayout->addWidget(addSemTag,linecounter,2);
}

QJsonObject QGMAnnotationDialog::saveAnnotationJSON(){
    QJsonObject result=QJsonObject();
    result.insert("body",QJsonArray());
    QJsonArray body=result.value("body").toArray();
    auto *curlayout = dynamic_cast<QGridLayout *>(this->tabw->currentWidget()->layout());
    for(int i=0;i<curlayout->rowCount();i++){
        QLabel lab= (const QLabel) curlayout->itemAtPosition(i, 0)->widget();
        QLineEdit edit= (const QLineEdit) curlayout->itemAtPosition(i, 1)->widget();
        body.append(QJsonObject());
        body.at(i).toObject().insert("type","TextualBody");
        body.at(i).toObject().insert("purpose",lab.text().replace(":",""));
        body.at(i).toObject().insert("value",edit.text());
        body.at(i).toObject().insert("source",lab.text().replace(":",""));
    }
    return result;
}

void QGMAnnotationDialog::createInputFieldByType(const QString& inputtype,int linecounter,const QString& key,QGridLayout* gridLayout,QWidget* curwidget,const QJsonObject& data,bool hasdata,QJsonArray curanno){
    QRegExp numberregex("\b[0-9]+\b");
    numberregex.setCaseSensitivity(Qt::CaseInsensitive);
    numberregex.setPatternSyntax(QRegExp::RegExp);
    QHash<QString,QString> valmap=getValuesFromAnnotation(curanno);
    if(inputtype=="text" || inputtype=="number"){
        auto * label = new QLabel(curwidget);
        label->setText(key+":");
        QRegExpValidator regValidator( numberregex, 0 );
        auto * edit =new QLineEdit(curwidget);
        if(valmap.contains(key)){
            edit->setText(valmap.value(key));
        }else{
            edit->setText("");
        }
        if(inputtype=="number"){
            edit->setValidator( &regValidator );
        }
        gridLayout->addWidget(label,linecounter,0);
        gridLayout->addWidget(edit,linecounter,1);
    }else if(inputtype=="select" && hasdata){
        auto * label = new QLabel(curwidget);
        label->setText(key+":");
        auto * cbox=new QComboBox(curwidget);
        int selectedindex=-1;
        QString selectedval=*new QString("");
        if(valmap.contains(key)){
            selectedval=valmap.value(key);
        }
        auto dkeys=data.keys();
        dkeys.sort();
        int i=0;
        for(const QString& dkey:dkeys){
            cbox->addItem(dkey);
            if(dkey==selectedval){
                selectedindex=i;
            }
            i++;
        }
        if(selectedindex!=-1){
            cbox->setCurrentIndex(selectedindex);
        }
        gridLayout->addWidget(label,linecounter,0);
        gridLayout->addWidget(cbox,linecounter,1);
    }else if(inputtype=="paleocodage"){
        auto * label = new QLabel(curwidget);
        label->setText(key+":");
        QRegExpValidator regValidator( numberregex, 0 );
        auto * edit =new QLineEdit(curwidget);
        if(valmap.contains(key)){
            edit->setText(valmap.value(key));
        }else{
            edit->setText("");
        }
        if(inputtype=="number"){
            edit->setValidator( &regValidator );
        }
        gridLayout->addWidget(label,linecounter,0);
        gridLayout->addWidget(edit,linecounter,1);
        /*auto svgw=new QSvgWidget(this);
        gridLayout->addWidget(svgw,linecounter,2);*/
    }
}