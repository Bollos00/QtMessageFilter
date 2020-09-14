#include "messagefilterqt.h"
#include <QDebug>

QtMessageFilter* QtMessageFilter::_singleton_instance = nullptr;

void QtMessageFilter::resetInstance(bool hide)
{
    delete QtMessageFilter::_singleton_instance;
    QtMessageFilter::_singleton_instance = new QtMessageFilter();

    if(!hide)
        QtMessageFilter::_instance()->showDialog();
}

void QtMessageFilter::releaseInstance()
{
    delete QtMessageFilter::_singleton_instance;
    QtMessageFilter::_singleton_instance = nullptr;
}

bool QtMessageFilter::good()
{
    return (bool)QtMessageFilter::_singleton_instance;
}

void QtMessageFilter::messageOutput(const QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if(QtMessageFilter::good())
        QtMessageFilter::_instance()->_message_output(type, context, msg);
//    else
    //        qDebug()<<msg;
}

void QtMessageFilter::hideDialog()
{
    QtMessageFilter::_instance()->hide();
}

void QtMessageFilter::showDialog()
{
    QtMessageFilter::_instance()->show();
}

void QtMessageFilter::moveFilterToThread(QThread* thread)
{
    QtMessageFilter::_instance()->moveToThread(thread);
}

QtMessageFilter::QtMessageFilter(QWidget *parent)
    : QDialog(parent),
      _debug(),
      _info(),
      _warning(),
      _critical(),
      _last_id(0),
      _list(),
      _scroll_area(new QScrollArea(this)),
      _widget_scroll_area(new QWidget()),
      _vertical_layout_scroll_area(new QVBoxLayout(_widget_scroll_area)),
      _horizontal_layout(new QHBoxLayout()),
      _horizontal_spacer(),
      _cb_debug(new QCheckBox(this)),
      _cb_info(new QCheckBox(this)),
      _cb_warning(new QCheckBox(this)),
      _cb_critical(new QCheckBox(this)),
      _vertical_layout_global(new QVBoxLayout()),
      _current_dialog(nullptr),
      _current_dialog_text(nullptr)

{

    _cb_debug->setFixedSize(20, 20);
    _cb_info->setFixedSize(20, 20);
    _cb_warning->setFixedSize(20, 20);
    _cb_critical->setFixedSize(20, 20);

    _cb_debug->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                             "QCheckBox::indicator::unchecked { image : url(:/share/icons/debug_off.png); }\n"
                             "QCheckBox::indicator::checked { image : url(:/share/icons/debug_on.png); }");

    _cb_info->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                            "QCheckBox::indicator::unchecked { image : url(:/share/icons/info_off.png); }\n"
                            "QCheckBox::indicator::checked { image : url(:/share/icons/info_on.png); }");

    _cb_warning->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                               "QCheckBox::indicator::unchecked { image : url(:/share/icons/warning_off.png); }\n"
                               "QCheckBox::indicator::checked { image : url(:/share/icons/warning_on.png); }");

    _cb_critical->setStyleSheet("QCheckBox::indicator { width : 20; height : 20; }\n"
                                "QCheckBox::indicator::unchecked { image : url(:/share/icons/critical_off.png); }\n"
                                "QCheckBox::indicator::checked { image : url(:/share/icons/critical_on.png); }");

    for(uchar i=0; i<5; i++)
    {
        _horizontal_spacer[i] = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    _horizontal_layout->addItem(_horizontal_spacer[0]);
    _horizontal_layout->addWidget(_cb_debug);
    _horizontal_layout->addItem(_horizontal_spacer[0]);
    _horizontal_layout->addWidget(_cb_info);
    _horizontal_layout->addItem(_horizontal_spacer[0]);
    _horizontal_layout->addWidget(_cb_warning);
    _horizontal_layout->addItem(_horizontal_spacer[0]);
    _horizontal_layout->addWidget(_cb_critical);
    _horizontal_layout->addItem(_horizontal_spacer[0]);


    _scroll_area->setWidgetResizable(true);
    _widget_scroll_area->setGeometry(0, 0, 400, 800);
    _widget_scroll_area->setLayout(_vertical_layout_scroll_area);
    _scroll_area->setWidget(_widget_scroll_area);



//    delete this->layout();
    _vertical_layout_global->addItem(_horizontal_layout);
    _vertical_layout_global->addWidget(_scroll_area);

    this->setLayout(_vertical_layout_global);

    this->setMaximumSize(800, 1200);

    this->setMinimumSize(400, 900);

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

QtMessageFilter::~QtMessageFilter()
{

}

QtMessageFilter* QtMessageFilter::_instance()
{
    if(!QtMessageFilter::good()){
        qWarning("MessagefilterQt::instance() is nullptr. Call MessagefilterQt::resetInstance()\n"
                 "before use any method of the class Singleton");

        QtMessageFilter::resetInstance();
    }
    return QtMessageFilter::_singleton_instance;

}

void QtMessageFilter::_message_output(const QtMsgType type,
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
    _vertical_layout_scroll_area->addWidget(item.get());
    item->show();

    // Is this the best way of doing it?
    connect(item.get(), &MessageItem::SIGNAL_pressed, [this, messageInfo]{_create_dialog_with_message_info(*messageInfo);});
//    connect(item, &MessageItem::SIGNAL_released, [this]{currentDialog.reset();});

    _last_id++;
    item.take();
}

void QtMessageFilter::_create_dialog_with_message_info(const MessageInfo& info)
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

void QtMessageFilter::_unset_message(const QtMsgType typeMssage)
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
void QtMessageFilter::_set_message(const QtMsgType typeMessage)
{
    // simplify this
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

                ulong id = i->get()->id;
                MessageItem* itemAfter = nullptr;
                for(auto n = _list.begin(); n!=_list.end(); ++n)
                {
                    if(n->first.get()->id > id)
                    {
                        itemAfter = n->second;
                        break;
                    }
                }

                if(itemAfter)
                    _vertical_layout_scroll_area->insertWidget(_vertical_layout_scroll_area->indexOf(itemAfter), item);
                else
                    _vertical_layout_scroll_area->addWidget(item);

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

                ulong id = i->get()->id;
                MessageItem* itemAfter = nullptr;
                for(auto n = _list.begin(); n!=_list.end(); ++n)
                {
                    if(n->first.get()->id > id)
                    {
                        itemAfter = n->second;
                        break;
                    }
                }

                if(itemAfter)
                    _vertical_layout_scroll_area->insertWidget(_vertical_layout_scroll_area->indexOf(itemAfter), item);
                else
                    _vertical_layout_scroll_area->addWidget(item);

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

                ulong id = i->get()->id;
                MessageItem* itemAfter = nullptr;
                for(auto n = _list.begin(); n!=_list.end(); ++n)
                {
                    if(n->first.get()->id > id)
                    {
                        itemAfter = n->second;
                        break;
                    }
                }

                if(itemAfter)
                    _vertical_layout_scroll_area->insertWidget(_vertical_layout_scroll_area->indexOf(itemAfter), item);
                else
                    _vertical_layout_scroll_area->addWidget(item);

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

                ulong id = i->get()->id;
                MessageItem* itemAfter = nullptr;
                for(auto n = _list.begin(); n!=_list.end(); ++n)
                {
                    if(n->first.get()->id > id)
                    {
                        itemAfter = n->second;
                        break;
                    }
                }

                if(itemAfter)
                    _vertical_layout_scroll_area->insertWidget(_vertical_layout_scroll_area->indexOf(itemAfter), item);
                else
                    _vertical_layout_scroll_area->addWidget(item);

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
