#ifndef MESSAGEFILTERQT_H
#define MESSAGEFILTERQT_H

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QDateTime>
#include <QSpacerItem>


///
/// \brief This struct contains all information of a log message
/// \details It os very similar to [QMessageLogContext](https://doc.qt.io/qt-5/qmessagelogcontext.html),
/// but it has some additional information (the id of the message for the class
/// QtMessageFilter and the time of generation of the message).
/// It also hold not just the context of the message but the message itself
///
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
                const ulong thatId,
                const QDateTime thatDateTime);

    ~MessageInfo();
};


///
/// \brief This the widget of an item of a message
/// It is inherited from QLabel and it has some particular behviours.
/// It has its background black on normal state, but it becomes blue
/// when it is clicked with the left button of the mouse, it continues
/// blue until the user realese the button, then a dialog appear with
/// all informations of the message. But if the user keep pressing it
/// for 0.5s, then the message will be copied to the clipboard. This
/// behaviour is defined on QtMessageFilter::f_message_output, on the
/// creation of the item. Also, if the user presses the item with the
/// right button of the mouse, the item will be deleted.
///
class MessageItem : public QLabel
{
    Q_OBJECT
public:
    explicit MessageItem(QWidget* parent = nullptr);
    ~MessageItem();

private:
    QScopedPointer<QTimer> m_tmr_pressed;

private slots:
    void mousePressEvent(QMouseEvent *e = nullptr);
    void mouseReleaseEvent(QMouseEvent *e = nullptr);

signals:
    void SIGNAL_leftButtonPressed();
    void SIGNAL_leftButtonReleased();
    void SIGNAL_rightButtonPressed();
};


///
/// \brief This class is responsible for treat the messages of the application
/// \details It is a Singleton class, because there must be only one instance
/// of this class. It can be initialed calling the function
/// QtMessageFilter::resetInstance(), with the option to choose the parent
/// widget. When it is initialed, it install the message handler of the
/// function QtMessageFilter::f_message_filter(). It is generated a new User
/// Interface showing a list all the messages that are generated on the execution of
/// the application.
///
/// It is possible to distinguish the message type by its font color.
/// * Debug messages are cyan;
/// * Info messages are light green;
/// * Warning messages are yellow;
/// * Critical messages are red.
///
/// For information on how to generate those messages, check those links:
/// [https://doc.qt.io/qt-5/qmessagelogger.html] and
/// [https://doc.qt.io/qt-5/qtglobal.html#QtMessageHandler-typedef].
///
/// It is possible to omit or show some specific type of message by
/// selecting the checkboxess on the top of the Dialog.
///
/// It is possible to show and hide the User Interface calling the functions
/// QtMessageFilter::hideDialog and
/// QtMessageFilter::showDialog.
///
/// Note that this class will be operating even when it is hidden. To delete the instance
/// of the class and disable the message filter, call QtMessageFilter::releaseInstance().
///
/// One last recurse of this class is a log file that is generated containing all the
/// messages -- with its informations -- of the last session. Note that the log file
/// always overwrite the one of the last file, so save a copy if needed.
///
class QtMessageFilter : public QDialog
{
    Q_OBJECT

public:

    static void resetInstance(QWidget* parent = nullptr, bool hideDialog = false);
    static void releaseInstance();
    static bool good();


    static void hideDialog();
    static void showDialog();
    static bool isDialogVisible();
    static void setInstanceParent(QWidget* parent);

protected:

    void closeEvent(QCloseEvent *event = nullptr);
    void reject();


private:

    QtMessageFilter(QWidget *parent = nullptr);
    QtMessageFilter(const QtMessageFilter& that) = delete;
    QtMessageFilter(QtMessageFilter&& that) = delete;
    ~QtMessageFilter();

    static QtMessageFilter* f_instance();
    static QtMessageFilter* m_singleton_instance;

    void f_configure_ui();

    static void f_message_filter(const QtMsgType type,
                                 const QMessageLogContext& context,
                                 const QString& msg);

    void f_message_output(const QtMsgType type,
                          const QMessageLogContext& context,
                          const QString& msg);

    void f_create_dialog_with_message_info(const MessageInfo& _info);

    void f_unset_message(const QtMsgType typeMssage);
    void f_set_message(const QtMsgType typeMessage);


    QList<QSharedPointer<MessageInfo>> m_debug;
    QList<QSharedPointer<MessageInfo>> m_info;
    QList<QSharedPointer<MessageInfo>> m_warning;
    QList<QSharedPointer<MessageInfo>> m_critical;

    ulong m_last_id;

    QList<  QPair< QSharedPointer<MessageInfo>, MessageItem* >  > m_list;


    // Begin UI
    QVBoxLayout* m_vertical_layout_global;

    QScrollArea* m_scroll_area;
    QWidget* m_widget_scroll_area;
    QVBoxLayout* m_vertical_layout_scroll_area;

    QHBoxLayout* m_horizontal_layout;
    QSpacerItem* m_horizontal_spacer[5];
    QCheckBox* m_cb_debug;
    QCheckBox* m_cb_info;
    QCheckBox* m_cb_warning;
    QCheckBox* m_cb_critical;

    // End UI


    QDialog* m_current_dialog;
    QPlainTextEdit* m_current_dialog_text;


    QScopedPointer<QFile> m_log_file;


    const ulong m_maximum_itens_size;
    const ulong m_maximum_message_info_size;
};
#endif // MESSAGEFILTERQT_H
