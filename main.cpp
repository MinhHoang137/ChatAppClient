#include "friendhandlers.h"
#include "grouphandlers.h"
#include "header.h"
#include "winsockclient.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  // Khởi tạo file log cho Client
  initClientLog();

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("winSockClient",
                                           WinSockClient::getInstance());
  engine.rootContext()->setContextProperty("friendHandlers",
                                           FriendHandlers::getInstance());
  engine.rootContext()->setContextProperty("groupHandlers",
                                           GroupHandlers::getInstance());

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("Client", "Main");

  return app.exec();
}
