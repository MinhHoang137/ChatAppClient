#ifndef GROUPHANDLERS_H
#define GROUPHANDLERS_H

#include <QJsonObject>
#include <QObject>
#include <QQmlEngine>
#include <QVariantList>


class GroupHandlers : public QObject {
  Q_OBJECT
  QML_ELEMENT
  Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)
  Q_PROPERTY(
      QVariantList groupMessages READ groupMessages NOTIFY groupMessagesChanged)
  Q_PROPERTY(
      QVariantList groupMembers READ groupMembers NOTIFY groupMembersChanged)

public:
  static GroupHandlers *getInstance();
  explicit GroupHandlers(QObject *parent = nullptr);

  Q_INVOKABLE void fetchGroups();
  Q_INVOKABLE void createGroup(const QString &groupName);
  Q_INVOKABLE void addUserToGroup(int groupID, int userID);
  Q_INVOKABLE void removeUserFromGroup(int groupID, int userID);
  Q_INVOKABLE void leaveGroup(int groupID);
  Q_INVOKABLE void sendGroupMessage(const QString &content);
  Q_INVOKABLE void loadGroupMessages(int groupID);
  Q_INVOKABLE void loadGroupMembers(int groupID);

  QVariantList groups() const;
  QVariantList groupMessages() const;
  QVariantList groupMembers() const;

signals:
  void groupsChanged();
  void groupMessagesChanged();
  void groupMembersChanged();

  void groupsReceived(const QJsonObject &data);
  void groupMessageReceived(const QJsonObject &data);
  void groupMembersReceived(const QJsonObject &data);

private slots:
  void onGroupsReceived(const QJsonObject &data);
  void onGroupMessageReceived(const QJsonObject &data);
  void onGroupMembersReceived(const QJsonObject &data);

private:
  static GroupHandlers *m_instance;
  QVariantList m_groups;
  QVariantList m_groupMessages;
  QVariantList m_groupMembers;
};

#endif // GROUPHANDLERS_H
