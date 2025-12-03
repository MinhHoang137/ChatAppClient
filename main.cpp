#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "winsockclient.h"
#include "friendhandlers.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;


    engine.rootContext()->setContextProperty("winSockClient", WinSockClient::getInstance());
    engine.rootContext()->setContextProperty("friendHandlers", FriendHandlers::getInstance());

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Client", "Main");

    return app.exec();
}
