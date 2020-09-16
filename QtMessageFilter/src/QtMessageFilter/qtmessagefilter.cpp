#include "qtmessagefilter.h"
#include <QDebug>
#include <QThread>
#include <QShortcut>

QtMessageFilter* QtMessageFilter::m_singleton_instance = nullptr;

void QtMessageFilter::resetInstance(bool hide)
{
    delete QtMessageFilter::m_singleton_instance;
    QtMessageFilter::m_singleton_instance = new QtMessageFilter();

    if(!hide)
        QtMessageFilter::showDialog();

    qInstallMessageHandler(QtMessageFilter::f_message_filter);
}

void QtMessageFilter::releaseInstance()
{
    delete QtMessageFilter::m_singleton_instance;
    QtMessageFilter::m_singleton_instance = nullptr;

    qInstallMessageHandler(0);
}

bool QtMessageFilter::good()
{
    return (bool)QtMessageFilter::m_singleton_instance;
}


void QtMessageFilter::hideDialog()
{
    QtMessageFilter::f_instance()->hide();
}

void QtMessageFilter::showDialog()
{
    QtMessageFilter::f_instance()->show();
}

QtMessageFilter::QtMessageFilter(QWidget *parent)
    : QDialog(parent),
      m_debug(),
      m_info(),
      m_warning(),
      m_critical(),
      m_last_id(0),
      m_list(),
      m_scroll_area(new QScrollArea(this)),
      m_widget_scroll_area(new QWidget()),
      m_vertical_layout_scroll_area(new QVBoxLayout(m_widget_scroll_area)),
      m_horizontal_layout(new QHBoxLayout()),
      m_horizontal_spacer(),
      m_cb_debug(new QCheckBox(this)),
      m_cb_info(new QCheckBox(this)),
      m_cb_warning(new QCheckBox(this)),
      m_cb_critical(new QCheckBox(this)),
      m_vertical_layout_global(new QVBoxLayout()),
      m_current_dialog(nullptr),
      m_current_dialog_text(nullptr),
      m_log_file(new QFile()),
      m_maximum_itens_size(100),
      m_maximum_message_info_size(100)
{
    f_configure_ui();

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this),
            &QShortcut::activated,
            this,
            &QWidget::hide);


    m_log_file->setFileName("QtMessageFilterLog.txt");
    if(m_log_file->remove())
        m_log_file->setFileName("QtMessageFilterLog.txt");
    m_log_file->open(QIODevice::WriteOnly);
}

// Crashes on destructor
QtMessageFilter::~QtMessageFilter()
{

}

QtMessageFilter* QtMessageFilter::f_instance()
{
    if(!QtMessageFilter::good()){
        qWarning("MessagefilterQt::instance() is nullptr. Call MessagefilterQt::resetInstance()\n"
                 "before use any method of the class Singleton");

        QtMessageFilter::resetInstance();
    }
    return QtMessageFilter::m_singleton_instance;

}

void QtMessageFilter::f_configure_ui()
{
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

    for(uchar i=0; i<5; i++)
    {
        m_horizontal_spacer[i] = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    m_horizontal_layout->addItem(m_horizontal_spacer[0]);
    m_horizontal_layout->addWidget(m_cb_debug);
    m_horizontal_layout->addItem(m_horizontal_spacer[0]);
    m_horizontal_layout->addWidget(m_cb_info);
    m_horizontal_layout->addItem(m_horizontal_spacer[0]);
    m_horizontal_layout->addWidget(m_cb_warning);
    m_horizontal_layout->addItem(m_horizontal_spacer[0]);
    m_horizontal_layout->addWidget(m_cb_critical);
    m_horizontal_layout->addItem(m_horizontal_spacer[0]);


    m_scroll_area->setWidgetResizable(true);
    m_widget_scroll_area->setGeometry(0, 0, 400, 800);
    m_widget_scroll_area->setLayout(m_vertical_layout_scroll_area);
    m_scroll_area->setWidget(m_widget_scroll_area);



//    delete this->layout();
    m_vertical_layout_global->addItem(m_horizontal_layout);
    m_vertical_layout_global->addWidget(m_scroll_area);

    this->setLayout(m_vertical_layout_global);

    this->setMaximumSize(800, 1200);

    this->setMinimumSize(400, 900);

    // This looks unecessary, I shall investigate another option
    connect(m_cb_debug, &QCheckBox::released,
            [this]{if(m_cb_debug->isChecked()) f_set_message(QtDebugMsg); else f_unset_message(QtDebugMsg);});
    connect(m_cb_info, &QCheckBox::released,
            [this]{if(m_cb_info->isChecked()) f_set_message(QtInfoMsg); else f_unset_message(QtInfoMsg);});
    connect(m_cb_warning, &QCheckBox::released,
            [this]{if(m_cb_warning->isChecked()) f_set_message(QtWarningMsg); else f_unset_message(QtWarningMsg);});
    connect(m_cb_critical, &QCheckBox::released,
            [this]{if(m_cb_critical->isChecked()) f_set_message(QtCriticalMsg); else f_unset_message(QtCriticalMsg);});

    m_cb_debug->setChecked(true);
    m_cb_info->setChecked(true);
    m_cb_warning->setChecked(true);
    m_cb_critical->setChecked(true);

    this->setWindowTitle("Qt Message Filter");
}

void QtMessageFilter::f_message_filter(const QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if(QtMessageFilter::good())
        QtMessageFilter::f_instance()->f_message_output(type, context, msg);
//    else
    //        qDebug()<<msg;
}

void QtMessageFilter::f_message_output(const QtMsgType type,
                                      const QMessageLogContext& context,
                                      const QString& msg)
{
    // Limit the size of the lists and create Log File of messages

    QScopedPointer<MessageItem> item( new MessageItem(this) );
    QSharedPointer<MessageInfo> messageInfo( new MessageInfo(type, context, msg, m_last_id, QDateTime::currentDateTime()) );

    QTextStream streamLog(m_log_file.get());

    streamLog << "<<<<<<<<<<<<<<<" << messageInfo->id << "<<<<<<<<<<<<<<<\n";

    streamLog << "\\origin:\n" <<
                 messageInfo->fileName << " " << QString::number(messageInfo->line) << '\n' << '\n' <<

                 "\\function_call:\n" <<
                 messageInfo->function << '\n' << '\n' <<

                 "\\category:\n" <<
                 messageInfo->category << '\n' << '\n' <<

                 "\\time_date:\n" <<
                 messageInfo->dateTime.toString(Qt::ISODate) << '\n' << '\n';

    switch (type)
    {
        case QtDebugMsg:
            streamLog << "\\debug\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_debug.append(messageInfo);
            if((ulong)m_debug.size() > m_maximum_message_info_size)
                m_debug.removeFirst();

            if(!m_cb_debug->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : cyan; }");
            break;

        case QtInfoMsg:
            streamLog << "\\info\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_info.append(messageInfo);
            if((ulong)m_info.size() > m_maximum_message_info_size)
                m_info.removeFirst();

            if(!m_cb_info->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : #90ee90; }"); // light green font color
            break;

        case QtWarningMsg:
            streamLog << "\\warning\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_warning.append(messageInfo);
            if((ulong)m_warning.size() > m_maximum_message_info_size)
                m_warning.removeFirst();

            if(!m_cb_warning->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : yellow; }");
            break;

        case QtCriticalMsg:
            streamLog << "\\critical\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            m_critical.append(messageInfo);
            if((ulong)m_critical.size() > m_maximum_message_info_size)
                m_warning.removeFirst();

            if(!m_cb_critical->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : red; }");
            break;

        case QtFatalMsg:
            streamLog << "\\fatal\\id" << messageInfo->id << ": \n" <<
                         messageInfo->message + '\n';
            streamLog << ">>>>>>>>>>>>>>>" << messageInfo->id << ">>>>>>>>>>>>>>>\n";

            qFatal("%s", QString("Fatal: " + msg).toLatin1().data());
            return; // Note: never gets here, end on fatal (line above)
    }

    item->setText(msg);

    m_list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(messageInfo, item.get()));
    item->adjustSize();
    m_vertical_layout_scroll_area->addWidget(item.get());
    item->show();
    // Is this the best way of doing it?
    connect(item.get(), &MessageItem::SIGNAL_pressed, [this, messageInfo]{f_create_dialog_with_message_info(*messageInfo);});
//    connect(item, &MessageItem::SIGNAL_released, [this]{currentDialog.reset();});
    m_last_id++;
    item.take();

    if((ulong)m_vertical_layout_scroll_area->count() > m_maximum_itens_size)
        delete m_vertical_layout_scroll_area->takeAt(0);
}

void QtMessageFilter::f_create_dialog_with_message_info(const MessageInfo& info)
{
    if(!m_current_dialog)
    {
        m_current_dialog.reset(new QDialog());
        m_current_dialog_text.reset(new QPlainTextEdit(m_current_dialog.get()));
        m_current_dialog_text->setReadOnly(true);
    }

    QString typeStr;
    if(info.type == QtDebugMsg)
        typeStr = "Debug";
    else if(info.type == QtInfoMsg)
        typeStr = "Info";
    else if(info.type == QtWarningMsg)
        typeStr = "Warning";
    else if(info.type == QtCriticalMsg)
        typeStr = "Critical";

    m_current_dialog_text->setPlainText
            (
                "Origin : \n" +
                info.fileName + " " + QString::number(info.line) + '\n' + '\n' +

                "Function Call: \n" +
                info.function + '\n' + '\n' +

                "Category: \n" +
                info.category + '\n' + '\n' +

                "Time: " + '\n' +
                info.dateTime.toString(Qt::ISODate) + '\n' + '\n' +

                typeStr + " message " + QString::number(info.id) + ": \n" +
                info.message

             );

    m_current_dialog_text->adjustSize();
    m_current_dialog->adjustSize();
    m_current_dialog->show();
}

void QtMessageFilter::f_unset_message(const QtMsgType typeMssage)
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
void QtMessageFilter::f_set_message(const QtMsgType typeMessage)
{
    // simplify this
    QString styleSheet;
    QList<QSharedPointer<MessageInfo>>* listOfMessageType = nullptr;

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


    for(auto i = listOfMessageType->end();
        (ulong)m_vertical_layout_scroll_area->count() <= m_maximum_itens_size &&
        i!=listOfMessageType->begin(); )
    {
        --i;
        MessageItem* item = new MessageItem(this);
        item->setStyleSheet(styleSheet);
        item->setText(i->get()->message);
        m_list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(*i, item));
        item->adjustSize();

        const ulong id = i->get()->id;
        MessageItem* itemAfter = nullptr;
        for(auto n = m_list.begin(); n!=m_list.end(); ++n)
        {
            if(n->first.get()->id > id)
            {
                itemAfter = n->second;
                break;
            }
        }

        if(itemAfter)
            m_vertical_layout_scroll_area->insertWidget(m_vertical_layout_scroll_area->indexOf(itemAfter), item);
        else
            m_vertical_layout_scroll_area->addWidget(item);

        item->show();

        // Is this the best way of doing it?
        connect(item, &MessageItem::SIGNAL_pressed, [this, i]{f_create_dialog_with_message_info(*(i->get()));});
    }
}


MessageItem::MessageItem(QWidget* parent)
{

}

void MessageItem::mousePressEvent(QMouseEvent* e)
{
    this->setStyleSheet(this->styleSheet().replace("background-color : black", "background-color : blue", Qt::CaseInsensitive));
    emit SIGNAL_pressed();
}

void MessageItem::mouseReleaseEvent(QMouseEvent* e)
{
    this->setStyleSheet(this->styleSheet().replace("background-color : blue", "background-color : black", Qt::CaseInsensitive));
    emit SIGNAL_released();
}
