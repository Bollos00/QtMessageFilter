// MIT License

// Copyright (c) 2020-2021  Bruno Bollos Correa

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "QtMessageFilter/qtmessagefilter.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>


// Set there the maximum number of items
const int MAXIMUM_ITEMS_SIZE = 100;

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
                this, [this]
        {
           if(QtMessageFilter::good())
               QtMessageFilter::releaseInstance();
           else
               QtMessageFilter::resetInstance(this, false, MAXIMUM_ITEMS_SIZE);
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
    QtMessageFilter::resetInstance(w, true, MAXIMUM_ITEMS_SIZE);

    QTimer* tmr = new QTimer();
    QObject::connect(tmr, &QTimer::timeout,
                     w, []
    {
        static int k=0;

//        if(k == 7)
//        {
//            qFatal("Fatal error");
//        }
//        else
        if(k%4 == 0)
        {
            qDebug()<<"Teste "<< k;
        }
        else if(k%4 == 1)
        {
            qInfo()<<"Teste "<< k<<'\n'<<"A great line of text, but a big, very and very big line of text, lot of characteres in one single line";
        }
        else if(k%4 == 2)
        {
            qWarning()<<"Teste "<< k<<'\n'<<"Another line here";
        }
        else
        {
            qCritical()<<"Teste "<< k;
        }

        k++;
    });

    tmr->start(500);


    return a.exec();
}
