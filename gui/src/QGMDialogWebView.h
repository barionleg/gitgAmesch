//
// Created by timo on 1/26/23.
//

#ifndef GIGAMESH_QGMDIALOGWEBVIEW_H
#define GIGAMESH_QGMDIALOGWEBVIEW_H

#include <QtGui>
#include <QDialog>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEngineDownloadItem>

// Qt Interface includes:
#include "ui_dialogWebView.h"

// Qt includes:
#include "QGMMacros.h"

class QGMDialogWebView : public QDialog, private Ui::dialogWebView {
    Q_OBJECT

QString htmltemplate="<html><head><link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-alpha.5/css/bootstrap.min.css\" integrity=\"sha384-AysaV+vQoT3kOAXZkl02PThvDr8HYKPZhNT5h/CXfBThSRXQ6jW5DO2ekP5ViFdi\" crossorigin=\"anonymous\">\n"
                     "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/@recogito/annotorious-openseadragon/dist/annotorious.min.css\"></link>\n"
                     "<script src=\"https://code.jquery.com/jquery-1.12.4.js\"></script>\n"
                     "<script src=\"https://code.jquery.com/ui/1.12.1/jquery-ui.js\"></script><script type=\"text/javascript\" src=\"https://cdnjs.cloudflare.com/ajax/libs/openseadragon/3.0.0/openseadragon.min.js\"></script><script type=\"text/javascript\" src=\"https://cdn.jsdelivr.net/npm/@recogito/annotorious-openseadragon@2.7.4/dist/openseadragon-annotorious.min.js\"></script>\n"
                     "<script src=\"https://cdn.jsdelivr.net/npm/@recogito/annotorious-selector-pack@latest/dist/annotorious-selector-pack.min.js\"></script>\n"
                     "<script src=\"https://cdn.jsdelivr.net/npm/@recogito/annotorious-toolbar@latest/dist/annotorious-toolbar.min.js\"></script>\n"
                     "<script src=\"https://cdn.jsdelivr.net/npm/@recogito/annotorious-better-polygon@latest/dist/annotorious-better-polygon.js\"></script></head><body><h1>Cuneiform Annotator for image {{imagepath}}</h1>    <div id=\"my-toolbar-container\" style=\"background-color:white;position:relative;height:5%\"><span style=\"text-align:right;position:absolute;right:0px\">\n"
                     "<button id=\"leftimage2\" style=\"border-radius: 0 3px 3px 0;\" onclick=\"loadLeftRendering()\">&lt;&lt;</button>\n"
                     "<button id=\"rightimage2\" onclick=\"loadRightRendering()\" style=\"border-radius:3px 0 0 3px;\">&gt;&gt;</button><button id=\"showIndexedChars\" onClick=\"highlightIndexedChars()\">Indexview</button><button id=\"fatcrossview\" onclick=\"arrangeFatCross()\">FC View</button><button onclick=\"showHideAnno()\">Toggle</button><button id=\"saveInGit\" onclick=\"download(JSON.stringify(anno.getAnnotations()),'anno','json')\">Save</button></span><span id=\"saveannotationsmessage\" style=\"position:absolute;text-align:center;margin-top:40px;margin-right:80px;right:0px;\"></span><br/></div><div id=\"my-toolbar-container2\" style=\"background-color:white;position:relative;height:5%\"></div>\n"
                     "<div id=\"openseadragon\" ></div>\n"
                     "<a id=\"metadata\" onClick=\"getEXIFData('openseadragon','metadatafield')\">\n"
                     "<svg width=\"1em\" height=\"1em\" viewBox=\"0 0 16 16\" class=\"bi bi-info-circle-fill\" fill=\"currentColor\" xmlns=\"http://www.w3.org/2000/svg\"><path fill-rule=\"evenodd\" d=\"M8 16A8 8 0 1 0 8 0a8 8 0 0 0 0 16zm.93-9.412l-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM8 5.5a1 1 0 1 0 0-2 1 1 0 0 0 0 2z\"></path></svg>\n"
                     "</a><script> function download(data, filename, type) {\n"
                     "    var file = new Blob([data], {type: type});\n"
                     "    if (window.navigator.msSaveOrOpenBlob) // IE10+\n"
                     "        window.navigator.msSaveOrOpenBlob(file, filename);\n"
                     "    else { // Others\n"
                     "        var a = document.createElement(\"a\"),\n"
                     "                url = URL.createObjectURL(file);\n"
                     "        a.href = url;\n"
                     "        a.download = filename;\n"
                     "        document.body.appendChild(a);\n"
                     "        a.click();\n"
                     "        setTimeout(function() {\n"
                     "            document.body.removeChild(a);\n"
                     "            window.URL.revokeObjectURL(url);  \n"
                     "        }, 0); \n"
                     "    }\n"
                     "}"
                     "  var curnamespace=\"http://purl.org/cuneiform/\"; var mlVocabulary=[{\"label\":\"Broken\",\"uri\":curnamespace+\"Broken\"},{\"label\":\"Character\",\"uri\":curnamespace+\"Character\"},{\"label\":\"Line\",\"uri\":curnamespace+\"Line\"},{\"label\":\"Image\",\"uri\":curnamespace+\"Image\"},{\"label\":\"Word\",\"uri\":curnamespace+\"Word\"},{\"label\":\"Seal\",\"uri\":curnamespace+\"Seal\"},{\"label\":\"Phrase\",\"uri\":curnamespace+\"Phrase\"},{\"label\":\"Erased\",\"uri\":curnamespace+\"Erased\"},{\"label\":\"StrikeOut\",\"uri\":curnamespace+\"StrikeOut\"},{\"label\":\"Wordstart\",\"uri\":curnamespace+\"Wordstart\"},{\"label\":\"Wordend\",\"uri\":curnamespace+\"Wordend\"},{\"label\":\"InWord\",\"uri\":curnamespace+\"InWord\"},{\"label\":\"Wedge\",\"uri\":curnamespace+\"Wedge\"},{\"label\":\"UnknownIfWord\",\"uri\":curnamespace+\"UnknownIfWord\"}];\n"
                     " modtsources=[{\"type\":\"image\",\"url\":\"{{imagepath}}\"}];   "
                     "viewer = OpenSeadragon({\n"
                     "      id: \"openseadragon\",\n"
                     "      prefixUrl: \"https://cdnjs.cloudflare.com/ajax/libs/openseadragon/4.0.0/images/\",\n"
                     "      degrees: 0,\n"
                     "      autoHideControls: true,\n"
                     "      showRotationControl: true,\n"
                     "      showFlipControl: true,\n"
                     "      //sequenceMode: true,\n"
                     "      //showReferenceStrip:true,\n"
                     "      toolbar: \"my-toolbar-container\",\n"
                     "      gestureSettingsTouch: {\n"
                     "        pinchRotate: true\n"
                     "      },\n"
                     "      tileSources: modtsources\n"
                     "    });                           var annoconfig={\n"
                     "                            widgets:[  \n"
                     "                            //TextMapWidget,\n"
                     "                            'COMMENT',{ widget: 'TAG', \n"
                     "                            vocabulary: mlVocabulary\n"
                     "                            }\n"
                     "                            ]}\n"
                     "                             anno = OpenSeadragon.Annotorious(viewer,annoconfig);\n"
                     "                            //annotations = OpenSeadragon.Annotations({ viewer });\n"
                     "                            Annotorious.BetterPolygon(anno);\n"
                     "                            Annotorious.SelectorPack(anno);\n"
                     "                            Annotorious.Toolbar(anno, document.getElementById('my-toolbar-container2'));</script></body></html>";

public:
    QGMDialogWebView( QWidget *parent = nullptr);
    bool loadImageInWebView(QWidget *parent = nullptr, QString imagepath="");
    QWebEngineView *webView;
signals:

public slots:


    // QWidget interface
protected:

    void downloadRequested(QWebEngineDownloadItem *download);
};

#endif //GIGAMESH_QGMDIALOGWEBVIEW_H
