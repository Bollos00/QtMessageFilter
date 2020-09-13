#include "messagefilterqt.h"
#include <QDebug>

MessagefilterQt* MessagefilterQt::_singleton_instance = nullptr;

void MessagefilterQt::resetInstance(bool hide)
{
    delete MessagefilterQt::_singleton_instance;
    MessagefilterQt::_singleton_instance = new MessagefilterQt();

    if(!hide)
        MessagefilterQt::_instance()->showDialog();
}

void MessagefilterQt::releaseInstance()
{
    delete MessagefilterQt::_singleton_instance;
    MessagefilterQt::_singleton_instance = nullptr;
}

bool MessagefilterQt::good()
{
    return (bool)MessagefilterQt::_singleton_instance;
}

void MessagefilterQt::messageOutput(const QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if(MessagefilterQt::good())
        MessagefilterQt::_instance()->_message_output(type, context, msg);
//    else
    //        qDebug()<<msg;
}

void MessagefilterQt::hideDialog()
{
    MessagefilterQt::_instance()->hide();
}

void MessagefilterQt::showDialog()
{
    MessagefilterQt::_instance()->show();
}

void MessagefilterQt::moveFilterToThread(QThread* thread)
{
    MessagefilterQt::_instance()->moveToThread(thread);
}

MessagefilterQt::MessagefilterQt(QWidget *parent)
    : QDialog(parent),
      _debug(),
      _info(),
      _warning(),
      _critical(),
      _last_id(0),
      _list(),
      _scroll_area(new QScrollArea(this)),
      _layout_scroll_area(new QVBoxLayout()),
      _widget_scroll_area(new QWidget(this)),
      _cb_debug(new QCheckBox(this)),
      _cb_info(new QCheckBox(this)),
      _cb_warning(new QCheckBox(this)),
      _cb_critical(new QCheckBox(this)),
      _current_dialog(nullptr),
      _current_dialog_text(nullptr)

{
    _scroll_area->setWidgetResizable(true);
    _scroll_area->setGeometry(0, 20, 400, 800);
    this->resize(400, 820);

    _cb_debug->setGeometry(64, 0, 20, 20);
    _cb_info->setGeometry(148, 0, 20, 20);
    _cb_warning->setGeometry(232, 0, 20, 20);
    _cb_critical->setGeometry(316, 0, 20, 20);

    _cb_debug->setStyleSheet(QString("QCheckBox { background-color : cyan; }") + '\n' +
                           "QCheckBox::indicator {width: 20px; height: 20px;}");
    _cb_info->setStyleSheet(QString("QCheckBox { background-color : #90ee90; }") + '\n' +
                          "QCheckBox::indicator {width: 20px; height: 20px;}");
    _cb_warning->setStyleSheet(QString("QCheckBox { background-color : yellow; }") + '\n' +
                             "QCheckBox::indicator {width: 20px; height: 20px;}");
    _cb_critical->setStyleSheet(QString("QCheckBox { background-color : red; }") + '\n' +
                              "QCheckBox::indicator {width: 20px; height: 20px;}");

    _scroll_area->setWidget(_widget_scroll_area);

    _widget_scroll_area->setLayout(_layout_scroll_area);
//    this->layout()->addWidget(widgetScrollArea);

    // This looks unecessary, I shall investigate another option
    connect(_cb_debug, &QCheckBox::released,
            [this]{if(_cb_debug->isChecked()) _set_message(QtDebugMsg); else _unset_message(QtDebugMsg);});
    connect(_cb_info, &QCheckBox::released,
            [this]{if(_cb_info->isChecked()) _set_message(QtInfoMsg); else _unset_message(QtInfoMsg);});
    connect(_cb_warning, &QCheckBox::released,
            [this]{if(_cb_warning->isChecked()) _set_message(QtWarningMsg); else _unset_message(QtWarningMsg);});
    connect(_cb_critical, &QCheckBox::released,
            [this]{if(_cb_critical->isChecked()) _set_message(QtCriticalMsg); else _unset_message(QtCriticalMsg);});

    _cb_debug->setChecked(true);
    _cb_info->setChecked(true);
    _cb_warning->setChecked(true);
    _cb_critical->setChecked(true);
}

MessagefilterQt::~MessagefilterQt()
{

}

MessagefilterQt* MessagefilterQt::_instance()
{
    if(!MessagefilterQt::good()){
        qWarning("MessagefilterQt::instance() is nullptr. Call Singleton::resetInstance()\n"
                 "before use any method of the class Singleton");

        MessagefilterQt::resetInstance();
    }
    return MessagefilterQt::_singleton_instance;

}

void MessagefilterQt::_message_output(const QtMsgType type,
                                    const QMessageLogContext& context,
                                    const QString& msg)
{
    QScopedPointer<MessageItem> item( new MessageItem(this) );
    QSharedPointer<MessageInfo> messageInfo( new MessageInfo(type, context, msg, _last_id, QDateTime::currentDateTime()) );

    switch (type)
    {
        case QtDebugMsg:
            _debug.append(messageInfo);
//            qDebug()<<msg;
            if(!_cb_debug->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : cyan; }");
            break;

        case QtInfoMsg:
            _info.append(messageInfo);
            if(!_cb_info->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : #90ee90; }"); // light green font color
            break;

        case QtWarningMsg:
            _warning.append(messageInfo);
            if(!_cb_warning->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : yellow; }");
            break;

        case QtCriticalMsg:
            _critical.append(messageInfo);
            if(!_cb_critical->isChecked())
                return;
            item->setStyleSheet("QLabel { background-color : black; color : red; }");
            break;

        case QtFatalMsg:
            qFatal("%s", QString("Fatal: " + msg).toLatin1().data());
            return;
    }

    item->setText(msg);

    _list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(messageInfo, item.get()));
    item->adjustSize();
    _layout_scroll_area->addWidget(item.get());
    item->show();

    // Is this the best way of doing it?
    connect(item.get(), &MessageItem::SIGNAL_pressed, [this, messageInfo]{_create_dialog_with_message_info(*messageInfo);});
//    connect(item, &MessageItem::SIGNAL_released, [this]{currentDialog.reset();});

    _last_id++;
    item.take();
}

void MessagefilterQt::_create_dialog_with_message_info(const MessageInfo& info)
{
    if(!_current_dialog)
    {
        _current_dialog.reset(new QDialog());
        _current_dialog_text.reset(new QPlainTextEdit(_current_dialog.get()));
        _current_dialog_text->setReadOnly(true);
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

    _current_dialog_text->setPlainText
            (
                "Origin : \n" +
                info.fileName + " " + QString::number(info.line) + '\n' + '\n' +

                "Function Call: \n" +
                info.function + '\n' + '\n' +

                "Category: \n" +
                info.category + '\n' + '\n' +

                "id: " + QString::number(info.id) + '\n' + '\n' +

                "Time: " + '\n' +
                info.dateTime.toString(Qt::ISODate) + '\n' + '\n' +

                typeStr + " message: \n" +
                info.message

             );

    _current_dialog_text->adjustSize();
    _current_dialog->adjustSize();
    _current_dialog->show();
}

void MessagefilterQt::_unset_message(const QtMsgType typeMssage)
{
    for(auto i = _list.begin(); i!=_list.end();  )
    {
        if(i->first->type == typeMssage)
        {
            delete i->second;
            i = _list.erase(i);
        }
        else
        {
            ++i;
        }
    }
}
void MessagefilterQt::_set_message(const QtMsgType typeMessage)
{
    switch(typeMessage)
    {
        case QtDebugMsg:
        {
            QString styleSheet = "QLabel { background-color : black; color : cyan; }";
            for(auto i = _debug.begin(); i!=_debug.end(); ++i)
            {
                MessageItem* item = new MessageItem(this);
                item->setStyleSheet(styleSheet);
                item->setText(i->get()->message);
                _list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(*i, item));
                item->adjustSize();
                _layout_scroll_area->addWidget(item);
                item->show();

                // Is this the best way of doing it?
                connect(item, &MessageItem::SIGNAL_pressed, [this, i]{_create_dialog_with_message_info(*(i->get()));});
            }

        }return;

        case QtInfoMsg:
        {
            QString styleSheet = "QLabel { background-color : black; color : #90ee90; }";
            for(auto i = _info.begin(); i!=_info.end(); ++i)
            {
                MessageItem* item = new MessageItem(this);
                item->setStyleSheet(styleSheet);
                item->setText(i->get()->message);
                _list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(*i, item));
                item->adjustSize();
                _layout_scroll_area->addWidget(item);
                item->show();

                // Is this the best way of doing it?
                connect(item, &MessageItem::SIGNAL_pressed, [this, i]{_create_dialog_with_message_info(*(i->get()));});
            }

        }return;
        case QtWarningMsg:
        {
            QString styleSheet = "QLabel { background-color : black; color : yellow; }";
            for(auto i = _warning.begin(); i!=_warning.end(); ++i)
            {
                MessageItem* item = new MessageItem(this);
                item->setStyleSheet(styleSheet);
                item->setText(i->get()->message);
                _list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(*i, item));
                item->adjustSize();
                _layout_scroll_area->addWidget(item);
                item->show();

                // Is this the best way of doing it?
                connect(item, &MessageItem::SIGNAL_pressed, [this, i]{_create_dialog_with_message_info(*(i->get()));});
            }

        }return;
        case QtCriticalMsg:
        {
            QString styleSheet = "QLabel { background-color : black; color : red; }";
            for(auto i = _critical.begin(); i!=_critical.end(); ++i)
            {
                MessageItem* item = new MessageItem(this);
                item->setStyleSheet(styleSheet);
                item->setText(i->get()->message);
                _list.append(QPair< QSharedPointer<MessageInfo>, MessageItem* >(*i, item));
                item->adjustSize();
                _layout_scroll_area->addWidget(item);
                item->show();

                // Is this the best way of doing it?
                connect(item, &MessageItem::SIGNAL_pressed, [this, i]{_create_dialog_with_message_info(*(i->get()));});
            }

        }return;

        default:
            return;
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
