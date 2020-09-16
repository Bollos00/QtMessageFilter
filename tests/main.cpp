#include "QtMessageFilter/qtmessagefilter.h"

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

    QtMessageFilter::resetInstance();

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

//        if(k%20 == 9)
//            QtMessageFilter::hideDialog();

//        if(k%20 == 19)
//            QtMessageFilter::showDialog();

        k++;
    });

    tmr->start(500);


    return a.exec();
}
