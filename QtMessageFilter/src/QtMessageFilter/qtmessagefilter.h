//
// MIT License
//
// Copyright (c) 2020 Bollos00
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
/// \details It is very similar to [QMessageLogContext](https://doc.qt.io/qt-5/qmessagelogcontext.html),
/// but it has some additional information (the id of the message for the class
/// QtMessageFilter and the time of generation of the message). The id of the message
/// is count of messages when the message was generated, that way, the first message will
/// have id=0, the seconde one will have id=1 and so on.
/// It also hold not just the context of the message but the message itself.
///
struct MessageDetails
{
    const QtMsgType type;
    const int line;

    const QString fileName;
    const QString function;
    const QString category;

    const QString message;

    const ulong id;
    const QDateTime dateTime;

    MessageDetails(const QtMsgType thatType,
                   const QMessageLogContext& thatContext,
                   const QString& thatMessage,
                   const ulong thatId,
                   const QDateTime thatDateTime);

    ~MessageDetails();
};


///
/// \brief This the widget of an item of a message
/// \details It is inherited from QLabel and it has some particular behviours.
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
Q_DECLARE_METATYPE(QSharedPointer<MessageDetails>)

///
/// \brief This class is responsible for treat the messages of the application
/// \details It is a Singleton class, because there must be only one instance
/// of this class. It can be initialed calling the function
/// QtMessageFilter::resetInstance(), with the option to choose the parent
/// widget. When it is initialed, it install the message handler of the class.
/// It is generated a new User Interface showing a list all the messages
/// that are generated on the execution of the application.
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
/// selecting the checkboxess on the top of the Dialog. The blue spider
/// represents debug messages, the 'i' icon inside a green circle represents
/// information messages, the '!' inside a triangle represents warning
/// messages and the 'x' inside a red circle represents critical messages.
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
    void hideEvent(QHideEvent* event = nullptr);
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

    void f_create_dialog_with_message_details(const MessageDetails& details);

    void f_unset_message_of_type(const QtMsgType typeMessage);
    void f_set_message_of_type(const QtMsgType typeMessage);


    QList<QSharedPointer<MessageDetails>> m_debug;
    QList<QSharedPointer<MessageDetails>> m_info;
    QList<QSharedPointer<MessageDetails>> m_warning;
    QList<QSharedPointer<MessageDetails>> m_critical;

    ulong m_last_id;

    QList<  QPair< QSharedPointer<MessageDetails>, MessageItem* >  > m_list;


    // UI
    QVBoxLayout* m_vertical_layout_global;

    QScrollArea* m_scroll_area;
    QWidget* m_widget_scroll_area;
    QVBoxLayout* m_vertical_layout_scroll_area;

    QHBoxLayout* m_horizontal_layout;
    QSpacerItem* m_horizontal_spacer;
    QCheckBox* m_cb_debug;
    QCheckBox* m_cb_info;
    QCheckBox* m_cb_warning;
    QCheckBox* m_cb_critical;
    // UI


    // Dialog With message info
    QDialog* m_current_dialog;
    QVBoxLayout* m_current_dialog_vertical_layout;
    QPlainTextEdit* m_current_dialog_text;
    // Dialog With message info


    QScopedPointer<QFile> m_log_file;


    const ulong m_maximum_itens_size;
    const ulong m_maximum_message_details_size;

private Q_SLOTS:
    void slot_create_message_item(QSharedPointer<MessageDetails> messageDetails);

Q_SIGNALS:
    void signal_create_message_item(QSharedPointer<MessageDetails> messageDetails);
};
#endif // MESSAGEFILTERQT_H
