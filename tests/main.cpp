#include "QtMessageFilter/qtmessagefilter.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

class TestWidget : public QMainWindow
{
public:
    TestWidget(QWidget* parent = nullptr):
        QMainWindow(parent),
        l(new QLabel(this)),
        pb(new QPushButton(this))
    {
        l->setText("Label");
        l->adjustSize();
        l->move(0, 0);
        pb->setText("Close Message Filter");
        pb->adjustSize();
        pb->move(l->x() + l->width() + 5, 0);

        this->setWindowTitle("Main Window Test");

        connect(pb, &QPushButton::clicked,
                [this]
        {
            if(QtMessageFilter::isDialogVisible())
            {
                QtMessageFilter::hideDialog();
                pb->setText("Show Message Filter");
            }
            else
            {
                QtMessageFilter::showDialog();
                pb->setText("Hide Message Filter");
            }
        });

        this->setAttribute(Qt::WA_DeleteOnClose);
        this->show();
        this->adjustSize();
    }

    ~TestWidget()
    {
        qApp->quit();
    }

private:
    QLabel* l;
    QPushButton* pb;

};


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TestWidget* w = new TestWidget();
    QtMessageFilter::resetInstance(/*w*/);

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
