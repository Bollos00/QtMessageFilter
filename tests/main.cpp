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
        pb0(new QPushButton(this)),
        pb1(new QPushButton(this))
    {
        l->setText("Label");
        l->adjustSize();
        l->move(0, 0);
        pb0->setText("Show/hide Message Filter");
        pb0->adjustSize();
        pb0->move(l->x() + l->width() + 5, 0);
        pb1->setText("Switch Message Handler");
        pb1->adjustSize();
        pb1->move(pb0->x() + pb0->width() + 5, 0);
        this->resize(pb1->x() + pb1->width(), qMax(qMax(l->height(), pb0->height()), pb1->height()));

        this->setWindowTitle("Main Window Test");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->show();

        connect(pb0, &QPushButton::clicked,
                []
        {
            if(QtMessageFilter::isDialogVisible())
            {
                QtMessageFilter::hideDialog();
            }
            else
            {
                QtMessageFilter::showDialog();
            }
        });

        connect(pb1, &QPushButton::clicked,
                [this]
        {
           if(QtMessageFilter::good())
               QtMessageFilter::releaseInstance();
           else
               QtMessageFilter::resetInstance(this);
        });

    }

    ~TestWidget()
    {
//        qApp->quit();
    }

private:
    QLabel* l;
    QPushButton* pb0;
    QPushButton* pb1;
};


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TestWidget* w = new TestWidget();
    QtMessageFilter::resetInstance(w);

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
