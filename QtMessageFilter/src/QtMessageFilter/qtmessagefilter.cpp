//
// MIT License
//
// Copyright (c) 2020-2021  Bruno Bollos Correa
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


#include "qtmessagefilter.h"

#include <QDebug>
#include <QShortcut>
#include <QClipboard>
#include <QTimer>
#include <QApplication>
#include <QMutex>
#include <QScrollBar>

QtMessageFilter* QtMessageFilter::m_singleton_instance = nullptr;

// For multi-thread safe
QMutex qtMessageFilterMutex;

void QtMessageFilter::resetInstance(QWidget* parent, bool hide, const ulong maximumItensSize, const ulong maximumMessageDetailsSize)
{
    delete QtMessageFilter::m_singleton_instance;
    QtMessageFilter::m_singleton_instance = new QtMessageFilter(parent, maximumItensSize, maximumMessageDetailsSize);

    if(!hide)
        QtMessageFilter::showDialog();

    // Install the message handler of this class
    qInstallMessageHandler(QtMessageFilter::f_message_filter);
}

void QtMessageFilter::releaseInstance()
{
    if(!QtMessageFilter::good())
        return;

    delete QtMessageFilter::m_singleton_instance;
    QtMessageFilter::m_singleton_instance = nullptr;
}

bool QtMessageFilter::good()
{
    return (bool)QtMessageFilter::m_singleton_instance;
}

void QtMessageFilter::hideDialog()
{
    if(QtMessageFilter::good())
        QtMessageFilter::f_instance()->hide();
    else
    {
        qWarning()<<"You tried to call a method of the class QtMessageFilter when it was inactive,"
                    " please call QtMessageFilter::resetInstance before use any method of this class.\n"
                    "Thanks";
    }
}

void QtMessageFilter::showDialog()
{
    if(QtMessageFilter::good())
        QtMessageFilter::f_instance()->show();
    else
    {
        qWarning()<<"You tried to call a method of the class QtMessageFilter when it was inactive,"
                    " please call QtMessageFilter::resetInstance before use any method of this class.\n"
                    "Thanks";
    }
}

bool QtMessageFilter::isDialogVisible()
{
    if(!QtMessageFilter::good())
    {
        qWarning()<<"You tried to call a method of the class QtMessageFilter when it was inactive,"
                    " please call QtMessageFilter::resetInstance before use any method of this class.\n"
                    "Thanks.";
        return false;
    }

    return QtMessageFilter::f_instance()->isVisible();
}

void QtMessageFilter::setInstanceParent(QWidget* parent)
{
    if(QtMessageFilter::good())
        QtMessageFilter::f_instance()->setParent(parent);
    else
    {
        qWarning()<<"You tried to call a method of the class QtMessageFilter when it was inactive,"
                    " please call QtMessageFilter::resetInstance before use any method of this class.\n"
                    "Thanks.";
    }
}

void QtMessageFilter::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event)
    this->hide();
}

void QtMessageFilter::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event)
    m_current_dialog->hide();
    this->QWidget::hide();
}

void QtMessageFilter::reject()
{
    this->hide();
}

QtMessageFilter::QtMessageFilter(QWidget *parent, const ulong maximumItensSize, const ulong maximumMessageDetailsSize)
    : QDialog(parent),
      m_debug(),
      m_info(),
      m_warning(),
      m_critical(),
      m_last_id(0),
      m_list(),
      m_vertical_layout_global(new QVBoxLayout(this)),
      m_scroll_area(new QScrollArea(this)),
      m_widget_scroll_area(new QWidget()),
      m_vertical_layout_scroll_area(new QVBoxLayout(m_widget_scroll_area)),
      m_horizontal_layout(new QHBoxLayout()),
      m_horizontal_spacer(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum)),
      m_cb_debug(new QCheckBox(this)),
      m_cb_info(new QCheckBox(this)),
      m_cb_warning(new QCheckBox(this)),
      m_cb_critical(new QCheckBox(this)),
      m_current_dialog(new QDialog(this)),
      m_current_dialog_vertical_layout(new QVBoxLayout(m_current_dialog)),
      m_current_dialog_text(new QPlainTextEdit(m_current_dialog)),
      m_log_file(new QFile()),
      m_maximum_itens_size(maximumItensSize),
      m_maximum_message_details_size(maximumMessageDetailsSize)
{
    f_configure_ui();

    // Multi-thread support
    qRegisterMetaType<QSharedPointer<MessageDetails>>();

    // Multi-thread support, &QtMessageFilter::slot_create_message_item will be
    //  always executed on the main thread
    connect(this, &QtMessageFilter::signal_create_message_item,
            this, &QtMessageFilter::slot_create_message_item,
            Qt::QueuedConnection);

    // Remove last log file, current a new one and let it be opened
    m_log_file->setFileName("QtMessageFilterLog.txt");
    if(m_log_file->remove())
        m_log_file->setFileName("QtMessageFilterLog.txt");
    m_log_file->open(QIODevice::WriteOnly);

    // Log File Begin
    {
        QTextStream stream(m_log_file.get());
        stream << "\\BEGIN " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
               << "\n\n\n";
    }
}

QtMessageFilter::~QtMessageFilter()
{
    // Install the default message handler
    qInstallMessageHandler(0);

    // We have a little memory leak problem here, but without this
    //  line of code, the application crashes on destructor. Since
    //  we are ending the application at this point, it should not
    //  be a problem
    m_horizontal_layout->setParent(nullptr);

    // Log File End
    {
        QTextStream stream(m_log_file.get());
        stream << "\n\n\n"
               << "\\END " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    }
}

QtMessageFilter* QtMessageFilter::f_instance()
{
    if(!QtMessageFilter::good()){
        qWarning("MessagefilterQt::instance() is nullptr. Please call MessagefilterQt::resetInstance() "
                 "before use any method of the class Singleton.\nThanks.");

        QtMessageFilter::resetInstance();
    }
    return QtMessageFilter::m_singleton_instance;
}

void QtMessageFilter::f_configure_ui()
{
    // Constant size of the checkboxes
    m_cb_debug->setFixedSize(20, 20);
    m_cb_info->setFixedSize(20, 20);
    m_cb_warning->setFixedSize(20, 20);
    m_cb_critical->setFixedSize(20, 20);

    m_cb_debug->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                             "QCheckBox::indicator::unchecked { image : url(:/share/icons/debug_off.png); }\n"
                             "QCheckBox::indicator::checked { image : url(:/share/icons/debug_on.png); }");

    m_cb_info->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                            "QCheckBox::indicator::unchecked { image : url(:/share/icons/info_off.png); }\n"
                            "QCheckBox::indicator::checked { image : url(:/share/icons/info_on.png); }");

    m_cb_warning->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                               "QCheckBox::indicator::unchecked { image : url(:/share/icons/warning_off.png); }\n"
                               "QCheckBox::indicator::checked { image : url(:/share/icons/warning_on.png); }");

    m_cb_critical->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                                "QCheckBox::indicator::unchecked { image : url(:/share/icons/critical_off.png); }\n"
                                "QCheckBox::indicator::checked { image : url(:/share/icons/critical_on.png); }");

    m_horizontal_layout->addItem(m_horizontal_spacer);
    m_horizontal_layout->addWidget(m_cb_debug);
    m_horizontal_layout->addItem(m_horizontal_spacer);
    m_horizontal_layout->addWidget(m_cb_info);
    m_horizontal_layout->addItem(m_horizontal_spacer);
    m_horizontal_layout->addWidget(m_cb_warning);
    m_horizontal_layout->addItem(m_horizontal_spacer);
    m_horizontal_layout->addWidget(m_cb_critical);
    m_horizontal_layout->addItem(m_horizontal_spacer);




    m_scroll_area->setWidgetResizable(true);
    m_widget_scroll_area->setGeometry(0, 0, 400, 800);
    m_scroll_area->setWidget(m_widget_scroll_area);


    m_vertical_layout_global->addLayout(m_horizontal_layout);
    m_vertical_layout_global->addWidget(m_scroll_area);
    this->setLayout(m_vertical_layout_global);

    // Maximum and minimum sizes of the dialog
    this->setMaximumSize(800, 1600);
    this->setMinimumSize(400, 500);

    // This looks unecessary, I shall investigate another option
    // It connects the signals of the checkboxes on a lambda
    //  for omit or show the messages of that type, depending
    //  on the current state.
    // This signal is emmited whenever the state of the checkbox changes
    connect(m_cb_debug, &QCheckBox::stateChanged,
            [this]{ if(m_cb_debug->isChecked()) f_set_message_of_type(QtDebugMsg); else f_unset_message_of_type(QtDebugMsg); });
    connect(m_cb_info, &QCheckBox::stateChanged,
            [this]{ if(m_cb_info->isChecked()) f_set_message_of_type(QtInfoMsg); else f_unset_message_of_type(QtInfoMsg);} );
    connect(m_cb_warning, &QCheckBox::stateChanged,
            [this]{ if(m_cb_warning->isChecked()) f_set_message_of_type(QtWarningMsg); else f_unset_message_of_type(QtWarningMsg); });
    connect(m_cb_critical, &QCheckBox::stateChanged,
            [this]{ if(m_cb_critical->isChecked()) f_set_message_of_type(QtCriticalMsg); else f_unset_message_of_type(QtCriticalMsg); });

    // Connect shortcuts to change the state of the checkboxes
    connect(new QShortcut(QKeySequence(Qt::Key_D), this), &QShortcut::activated,
            [this]{ m_cb_debug->setChecked(!m_cb_debug->isChecked()); });
    connect(new QShortcut(QKeySequence(Qt::Key_I), this), &QShortcut::activated,
            [this]{ m_cb_info->setChecked(!m_cb_info->isChecked()); });
    connect(new QShortcut(QKeySequence(Qt::Key_W), this), &QShortcut::activated,
            [this]{ m_cb_warning->setChecked(!m_cb_warning->isChecked()); });
    connect(new QShortcut(QKeySequence(Qt::Key_C), this), &QShortcut::activated,
            [this]{ m_cb_critical->setChecked(!m_cb_critical->isChecked()); });



    // Initialize with all checkboxes checked
    m_cb_debug->setChecked(true);
    m_cb_info->setChecked(true);
    m_cb_warning->setChecked(true);
    m_cb_critical->setChecked(true);

    // Initialize dialog with message details
    m_current_dialog_vertical_layout->addWidget(m_current_dialog_text);
    m_current_dialog_text->setReadOnly(true);
    m_current_dialog->setWindowTitle("Message details");

    this->setWindowTitle("Qt Message Filter");
}

void QtMessageFilter::f_message_filter(const QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    qtMessageFilterMutex.lock();

    if(QtMessageFilter::good())
        QtMessageFilter::f_instance()->f_message_output(type, context, msg);

    qtMessageFilterMutex.unlock();
}

void QtMessageFilter::f_message_output(const QtMsgType type,
                                       const QMessageLogContext& context,
                                       const QString& msg)
{
    QSharedPointer<MessageDetails> messageInfo( new MessageDetails(type, context, msg, m_last_id++, QDateTime::currentDateTime()) );

    QTextStream streamLog(m_log_file.get());

    // Write message details on the log file
    streamLog << "<<<<<<<<<<<<<<<" << messageInfo->id << "<<<<<<<<<<<<<<<\n";
    streamLog << "\\origin:\n" <<
                 messageInfo->fileName << " " << QString::number(messageInfo->line) << '\n' << '\n' <<

                 "\\function_call:\n" <<
                 messageInfo->function << '\n' << '\n' <<

                 "\\category:\n" <<
                 messageInfo->category << '\n' << '\n' <<

                 "\\time_date:\n" <<
                 messageInfo->dateTime.toString(Qt::ISODateWithMs) << '\n' << '\n';

    switch (type)
    {
        case QtDebugMsg:
            streamLog << "\\debug\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_debug.append(messageInfo);
            if((ulong)m_debug.size() > m_maximum_message_details_size)
                m_debug.removeFirst();

            if(!m_cb_debug->isChecked())
                return;
            break;

        case QtInfoMsg:
            streamLog << "\\info\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_info.append(messageInfo);
            if((ulong)m_info.size() > m_maximum_message_details_size)
                m_info.removeFirst();

            if(!m_cb_info->isChecked())
                return;
            break;

        case QtWarningMsg:
            streamLog << "\\warning\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_warning.append(messageInfo);
            if((ulong)m_warning.size() > m_maximum_message_details_size)
                m_warning.removeFirst();

            if(!m_cb_warning->isChecked())
                return;
            break;

        case QtCriticalMsg:
            streamLog << "\\critical\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_critical.append(messageInfo);
            if((ulong)m_critical.size() > m_maximum_message_details_size)
                m_critical.removeFirst();

            if(!m_cb_critical->isChecked())
                return;
            break;

        case QtFatalMsg:
            streamLog << "\\fatal\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            qFatal("%s", QString("Fatal: " + msg).toLatin1().data());
            return; // Note: never gets here, end on fatal (line above)
    }


//    if(QThread::currentThread() == this->thread())
//    {
//        slot_create_message_item(messageInfo);
//    }
//    else
//    {
//        emit signal_create_message_item(messageInfo);
//    }

    emit signal_create_message_item(messageInfo);
}

void QtMessageFilter::f_create_dialog_with_message_details(const MessageDetails& details)
{
    QString typeStr;
    if(details.type == QtDebugMsg)
        typeStr = "Debug";
    else if(details.type == QtInfoMsg)
        typeStr = "Info";
    else if(details.type == QtWarningMsg)
        typeStr = "Warning";
    else if(details.type == QtCriticalMsg)
        typeStr = "Critical";

    m_current_dialog_text->setPlainText
            (
                "Origin:\n" +
                details.fileName + " " + QString::number(details.line) + '\n' + '\n' +

                "Function Call:\n" +
                details.function + '\n' + '\n' +

                "Category:\n" +
                details.category + '\n' + '\n' +

                "Time:\n" +
                details.dateTime.toString(Qt::ISODateWithMs) + '\n' + '\n' +

                typeStr + " message " + QString::number(details.id) + ":\n" +
                details.message

             );

    m_current_dialog->show();
}

void QtMessageFilter::f_unset_message_of_type(const QtMsgType typeMssage)
{
    for(auto i = m_list.begin(); i!=m_list.end();  )
    {
        if(i->first->type == typeMssage)
        {
            delete i->second;
            i = m_list.erase(i);
        }
        else
        {
            ++i;
        }
    }
}


void QtMessageFilter::f_set_message_of_type(const QtMsgType typeMessage)
{
    // simplify this
    QString styleSheet;
    QList<QSharedPointer<MessageDetails>>* listOfMessageType = nullptr;

    switch(typeMessage)
    {
        case QtDebugMsg:
        {
            styleSheet = "QLabel { background-color : black; color : cyan; }";
            listOfMessageType = &m_debug;
        }break;

        case QtInfoMsg:
        {
            styleSheet = "QLabel { background-color : black; color : #90ee90; }";
            listOfMessageType = &m_info;
        }break;
        case QtWarningMsg:
        {
            styleSheet = "QLabel { background-color : black; color : yellow; }";
            listOfMessageType = &m_warning;
        }break;
        case QtCriticalMsg:
        {
            styleSheet = "QLabel { background-color : black; color : red; }";
            listOfMessageType = &m_critical;
        }break;

        default:
            return;
    }

    // Iterate from the last element (added more recently) to the first
    for(auto i = listOfMessageType->end();
        (ulong)m_vertical_layout_scroll_area->count() <= m_maximum_itens_size &&
        i!=listOfMessageType->begin(); )
    {
        --i;
        QSharedPointer<MessageDetails> k = *i;
        MessageItem* item = new MessageItem(m_widget_scroll_area);
        item->setStyleSheet(styleSheet);
        item->setText(k->message);
        item->adjustSize();

        const ulong id = k->id;

        // Try to insert the new element on its right position, according with its id attribute
        MessageItem* itemAfter = nullptr;
        for(auto n = m_list.begin(); n!=m_list.end(); ++n)
        {
            if(n->first.get()->id > id)
            {
                itemAfter = n->second;
                m_list.insert(n, QPair< QSharedPointer<MessageDetails>, MessageItem* >(k, item));
                m_vertical_layout_scroll_area->insertWidget(m_vertical_layout_scroll_area->indexOf(itemAfter), item);
                break;
            }
        }
        if(!itemAfter)
        {
            m_vertical_layout_scroll_area->addWidget(item);
            m_list.append(QPair< QSharedPointer<MessageDetails>, MessageItem* >(k, item));
        }

        item->show();

        // Is this the best way of doing it?
        connect(item, &MessageItem::SIGNAL_leftButtonReleased,
                [this, k]{ f_create_dialog_with_message_details(*k); });
        connect(item, &MessageItem::SIGNAL_rightButtonPressed,
                [this, k, item]{ f_remove_item_from_list(k, item); });
    }
}

void QtMessageFilter::f_remove_item_from_list(QSharedPointer<MessageDetails> messageDetails, MessageItem* item)
{
    if(messageDetails.get()->type == QtDebugMsg)
        m_debug.removeOne(messageDetails);
    else if(messageDetails.get()->type == QtInfoMsg)
        m_info.removeOne(messageDetails);
    else if(messageDetails.get()->type == QtWarningMsg)
        m_warning.removeOne(messageDetails);
    else
        m_critical.removeOne(messageDetails);

    m_list.removeOne(QPair< QSharedPointer<MessageDetails>, MessageItem* >(messageDetails, item));
    item->disconnect();
    item->deleteLater();
}

void QtMessageFilter::slot_create_message_item(QSharedPointer<MessageDetails> messageDetails)
{
    QString styleSheet;

    switch(messageDetails->type)
    {
        case QtDebugMsg:
        {
            styleSheet = "QLabel { background-color : black; color : cyan; }";
        }break;
        case QtInfoMsg:
        {
            styleSheet = "QLabel { background-color : black; color : #90ee90; }";
        }break;
        case QtWarningMsg:
        {
            styleSheet = "QLabel { background-color : black; color : yellow; }";
        }break;
        case QtCriticalMsg:
        {
            styleSheet = "QLabel { background-color : black; color : red; }";
        }break;
        default:
            return;
    }

    MessageItem* item = new MessageItem(m_widget_scroll_area);

    item->setText(messageDetails->message);
    item->setStyleSheet(styleSheet);
    m_list.append(QPair< QSharedPointer<MessageDetails>, MessageItem* >(messageDetails, item));
    item->adjustSize();
    m_vertical_layout_scroll_area->addWidget(item);
    item->show();

    // Force update of the list of messages
    qApp->processEvents();

    // If the item further below is visible, make sure the new item continues visible as well
    const bool lockDownertical = m_scroll_area->verticalScrollBar()->maximum() - m_scroll_area->verticalScrollBar()->value() < 50;

    if(lockDownertical)
    {
        m_scroll_area->verticalScrollBar()->setValue(m_scroll_area->verticalScrollBar()->maximum());
    }

    // Is this the best way of doing it?
    connect(item, &MessageItem::SIGNAL_leftButtonReleased,
            [this, messageDetails]{f_create_dialog_with_message_details(*messageDetails);});
    connect(item, &MessageItem::SIGNAL_rightButtonPressed,
            [this, messageDetails, item]{ f_remove_item_from_list(messageDetails, item); });

    if((ulong)m_vertical_layout_scroll_area->count() > m_maximum_itens_size)
    {
        delete m_list.first().second;
        m_list.removeFirst();
    }
}


MessageItem::MessageItem(QWidget* parent):
    QLabel(parent),
    m_tmr_pressed(nullptr)
{}

MessageItem::~MessageItem()
{
    if(m_tmr_pressed && m_tmr_pressed->isActive())
        m_tmr_pressed->stop();
}

void MessageItem::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        this->setStyleSheet(this->styleSheet().replace("background-color : black", "background-color : blue", Qt::CaseInsensitive));

        m_tmr_pressed.reset(new QTimer());
        connect(m_tmr_pressed.get(),
                &QTimer::timeout,
                [this]
        {
            QApplication::clipboard()->setText(this->text());
            m_tmr_pressed->stop();
            m_tmr_pressed->disconnect();
            m_tmr_pressed.reset();
            this->setStyleSheet(this->styleSheet().replace("background-color : blue", "background-color : black", Qt::CaseInsensitive));
        });
        m_tmr_pressed->start(500);

        emit SIGNAL_leftButtonPressed();
    }
    else if(e->button() == Qt::RightButton)
    {
        emit SIGNAL_rightButtonPressed();
    }
}

void MessageItem::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(m_tmr_pressed)
        {
            m_tmr_pressed->stop();
            m_tmr_pressed->disconnect();
            m_tmr_pressed.reset();
            this->setStyleSheet(this->styleSheet().replace("background-color : blue", "background-color : black", Qt::CaseInsensitive));
            emit SIGNAL_leftButtonReleased();
        }
    }
}

MessageDetails::MessageDetails(const QtMsgType thatType,
                               const QMessageLogContext& thatContext,
                               const QString& thatMessage,
                               const ulong thatId,
                               const QDateTime thatDateTime) :
    type(thatType),
    line(thatContext.line),
    fileName(thatContext.file),
    function(thatContext.function),
    category(thatContext.category),
    message(thatMessage),
    id(thatId),
    dateTime(thatDateTime)
{

}

MessageDetails::~MessageDetails()
{

}
