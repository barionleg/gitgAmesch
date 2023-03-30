//
// Created by timo on 1/26/23.
//

#include "QGMDialogWebView.h"
#include <QUrl>
#include <QFileDialog>

QGMDialogWebView::QGMDialogWebView( QWidget *parent) : QDialog( parent) {
    //! Constructor
    setupUi( this );
    auto *webVieww  = new QWebEngineView(this);
    webVieww->load(QUrl("http://www.google.de"));
    this->webView=webVieww;
    this->gridLayout_2->addWidget(webView,1,0,1,0);
    this->loadImageInWebView(this,"https://heidicon.ub.uni-heidelberg.de/iiif/2/1667900:870974/full/full/0/default.jpg");
    //this->webView=QWebEngineView(this);
    // Set logo
    //QObject::connect(this->webView->page()->profile(), SIGNAL(downloadRequested(QWebEngineDownloadItem*)),
    //        this, SLOT(downloadRequested(QWebEngineDownloadItem*)));
    setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
    this->show();
}

void QGMDialogWebView::downloadRequested(QWebEngineDownloadItem* download) {
    Q_ASSERT(download && download->state() == QWebEngineDownloadItem::DownloadRequested);

    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), QDir(download->downloadDirectory()).filePath(download->downloadFileName()));
    if (path.isEmpty())
        return;

    download->setDownloadDirectory(QFileInfo(path).path());
    download->setDownloadFileName(QFileInfo(path).fileName());
    download->accept();
}

bool QGMDialogWebView::loadImageInWebView(QWidget *parent, QString imagepath){
    this->htmltemplate.replace("{{imagepath}}",imagepath);
    //QWebEnginePage page = QWebEnginePage(this->webView);
    this->webView->setHtml(htmltemplate);
    return true;
}