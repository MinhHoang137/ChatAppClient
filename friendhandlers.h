#ifndef FRIENDHANDLERS_H
#define FRIENDHANDLERS_H

#include <QObject>
#include <QJsonObject>
#include <QVariantList>
#include <QQmlEngine>

class FriendHandlers : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList nonFriendUsers READ nonFriendUsers NOTIFY nonFriendUsersChanged)

public:
    explicit FriendHandlers(QObject *parent = nullptr);

    Q_INVOKABLE void fetchNonFriendUsers();
    QVariantList nonFriendUsers() const;

signals:
    void nonFriendUsersChanged();

private slots:
    void onNonFriendUsersReceived(const QJsonObject &data);

private:
    QVariantList m_nonFriendUsers;
};

#endif // FRIENDHANDLERS_H
