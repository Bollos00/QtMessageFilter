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
#include <QFile>

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
