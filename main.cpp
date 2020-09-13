#include "messagefilterqt.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>

//void messageOutput(const QtMsgType type,
//                   const QMessageLogContext &context,
//                   const QString &msg){

//    if(r->good)
//        r->p_messageOutput(type, context, msg);
//    else
//        qDebug()<<msg;

//}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MessagefilterQt::resetInstance();
    qInstallMessageHandler(MessagefilterQt::messageOutput);

    {
        QPalette darkPalette;
        darkPalette.setColor (QPalette::BrightText,      Qt::red);
        darkPalette.setColor (QPalette::WindowText,      Qt::white);
        darkPalette.setColor (QPalette::ToolTipBase,     Qt::white);
        darkPalette.setColor (QPalette::ToolTipText,     Qt::white);
        darkPalette.setColor (QPalette::Text,            Qt::white);
        darkPalette.setColor (QPalette::ButtonText,      Qt::white);
        darkPalette.setColor (QPalette::HighlightedText, Qt::black);
        darkPalette.setColor (QPalette::Window,          QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Base,            QColor (25, 25, 25));
        darkPalette.setColor (QPalette::AlternateBase,   QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Button,          QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Link,            QColor (42, 130, 218));
        darkPalette.setColor (QPalette::Highlight,       QColor (42, 130, 218));
        qApp->setPalette (darkPalette);
    }

    QTimer* tmr = new QTimer();
    QObject::connect(tmr, &QTimer::timeout,
                     []
    {
        static int k=0;

        if(k%4 == 0)
            qDebug()<<"Teste "<< k;
        else if(k%4 == 1)
            qInfo()<<"Teste "<< k<<'\n'<<"A great line of text, but a big, very and very big line of text, lot of characteres in one single line";
        else if(k%4 == 2)
            qWarning()<<"Teste "<< k<<'\n'<<"Another line here";
        else
            qCritical()<<"Teste "<< k;

        if(k%20 == 9)
            MessagefilterQt::hideDialog();

        if(k%20 == 19)
            MessagefilterQt::showDialog();

        k++;
    });

    tmr->start(500);


    return a.exec();
}
