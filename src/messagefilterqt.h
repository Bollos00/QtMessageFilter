#ifndef MESSAGEFILTERQT_H
#define MESSAGEFILTERQT_H

#include <QList>
#include <QPointer>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QDateTime>
#include <QSpacerItem>
#include <QApplication>

struct MessageInfo
{
    const QtMsgType type;
    const int line;

    const QString fileName;
    const QString function;
    const QString category;

    const QString message;

    const ulong id;
    const QDateTime dateTime;

    MessageInfo(const QtMsgType thatType,
                const QMessageLogContext& thatContext,
                const QString& thatMessage,
                const unsigned long thatId,
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

    ~MessageInfo()
    {
//        qDebug(Q_FUNC_INFO);
    }
};

class MessageItem : public QLabel
{
    Q_OBJECT
public:
    explicit MessageItem(QWidget* parent = nullptr);

private slots:
    void mousePressEvent(QMouseEvent *e = nullptr);
    void mouseReleaseEvent(QMouseEvent *e = nullptr);

signals:
    void SIGNAL_pressed();
    void SIGNAL_released();
};

class QtMessageFilter : public QDialog
{
    Q_OBJECT

public:

    static void resetInstance(bool hideDialog = false);
    static void releaseInstance();
    static bool good();

    static void messageOutput(const QtMsgType type,
                              const QMessageLogContext& context,
                              const QString& msg);

    static void hideDialog();
    static void showDialog();
    static void moveFilterToThread(QThread* thread);

private:

    QtMessageFilter(QWidget *parent = nullptr);
    QtMessageFilter(const QtMessageFilter& that) = delete;
    QtMessageFilter(QtMessageFilter&& that) = delete;
    ~QtMessageFilter();;

    static QtMessageFilter* _instance();

    static QtMessageFilter* _singleton_instance;


    void _message_output(const QtMsgType type,
                         const QMessageLogContext& context,
                         const QString& msg);

    QList<QSharedPointer<MessageInfo>> _debug;
    QList<QSharedPointer<MessageInfo>> _info;
    QList<QSharedPointer<MessageInfo>> _warning;
    QList<QSharedPointer<MessageInfo>> _critical;

    ulong _last_id;

    QList<  QPair< QSharedPointer<MessageInfo>, MessageItem* >  > _list;


    // Begin UI
    QScrollArea* _scroll_area;
    QWidget* _widget_scroll_area;
    QVBoxLayout* _vertical_layout_scroll_area;

    QHBoxLayout* _horizontal_layout;
    QSpacerItem* _horizontal_spacer[5];
    QCheckBox* _cb_debug;
    QCheckBox* _cb_info;
    QCheckBox* _cb_warning;
    QCheckBox* _cb_critical;

    QVBoxLayout* _vertical_layout_global;
    // End UI


    QScopedPointer<QDialog> _current_dialog;
    QScopedPointer<QPlainTextEdit> _current_dialog_text;

    void _create_dialog_with_message_info(const MessageInfo& _info);

    void _unset_message(const QtMsgType typeMssage);
    void _set_message(const QtMsgType typeMessage);

};
#endif // MESSAGEFILTERQT_H
