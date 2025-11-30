#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "winsockclient.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    WinSockClient client;
    engine.rootContext()->setContextProperty("winSockClient", &client);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Client", "Main");

    return app.exec();
}
