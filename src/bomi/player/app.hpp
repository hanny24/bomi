#ifndef APP_HPP
#define APP_HPP

#include "tmp/type_test.hpp"

class QUrl;                             class Mrl;
class MainWindow;                       class QMenuBar;
class Locale;

class App : public QApplication {
    Q_OBJECT
public:
    enum MessageType {
        CommandLine = 1
    };
    App(int &argc, char **argv);
    ~App();
    auto setWindowTitle(QWidget *w, const QString &title) -> void;
    auto setMainWindow(MainWindow *mw) -> void;
    auto mainWindow() const -> MainWindow*;
    auto devices() const -> QStringList;
    auto styleName() const -> QString;
    auto isUnique() const -> bool;
    auto availableStyleNames() const -> QStringList;
#ifdef Q_OS_MAC
    QMenuBar *globalMenuBar() const;
#endif
    auto setStyleName(const QString &name) -> void;
    auto setLocale(const Locale &locale) -> void;
    auto locale() const -> Locale;
    auto setAlwaysOnTop(QWidget *widget, bool onTop) -> void;
    auto setScreensaverDisabled(bool disabled) -> void;
    auto setHeartbeat(const QString &command, int interval) -> void;
    auto setUnique(bool unique) -> void;
    auto shutdown() -> bool;
    auto runCommands() -> void;
    auto isOpenGLDebugLoggerRequested() const -> bool;
    auto setMprisActivated(bool activated) -> void;
    template<class T>
    auto sendMessage(MessageType type, const T &t, int timeout = 5000)
    -> tmp::enable_if_t<tmp::is_one_of<T, QJsonObject, QJsonArray, QJsonValue>(), bool>;
    static constexpr auto version() -> const char* { return "0.9.0"; }
    static constexpr auto name() -> const char* { return "bomi"; }
    static auto displayName() -> QString { return tr("bomi"); }
    static auto defaultIcon() -> QIcon;
private:
    auto sendMessage(const QByteArray &message, int timeout = 5000) -> bool;
    auto handleMessage(const QByteArray &message) -> void;
    static constexpr int ReopenEvent = QEvent::User + 1;
    auto event(QEvent *event) -> bool;
    struct Data;
    Data *d = nullptr;
};

#define cApp (*static_cast<App*>(qApp))

template<class T>
auto App::sendMessage(MessageType type, const T &t, int timeout)
-> tmp::enable_if_t<tmp::is_one_of<T, QJsonObject, QJsonArray, QJsonValue>(), bool>
{
    QJsonObject json;
    json[u"type"_q] = _JsonFromInt(type);
    json[u"contents"_q] = t;
    return sendMessage(QJsonDocument(json).toJson(), timeout);
}

#endif // APPLICATION_HPP
