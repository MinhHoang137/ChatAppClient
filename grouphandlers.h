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
      QVariantList groupMembers READ groupMembers NOTIFY groupMembersChanged)
  Q_PROPERTY(QVariantList allGroups READ allGroups NOTIFY allGroupsChanged)
  Q_PROPERTY(
      QVariantList groupRequests READ groupRequests NOTIFY groupRequestsChanged)
  Q_PROPERTY(
      QString exploreMessage READ exploreMessage NOTIFY exploreMessageChanged)

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

  Q_INVOKABLE void exploreGroups();
  Q_INVOKABLE void requestJoinGroup(int groupID);
  Q_INVOKABLE void getGroupRequests(int groupID);
  Q_INVOKABLE void respondToRequest(int requestID, int status);

  QVariantList groups() const;
  QVariantList groupMessages() const;
  QVariantList groupMembers() const;
  QVariantList allGroups() const;
  QVariantList groupRequests() const;
  QString exploreMessage() const;

signals:
  void groupsChanged();
  void groupMessagesChanged();
  void groupMembersChanged();
  void allGroupsChanged();
  void groupRequestsChanged();
  void exploreMessageChanged();
  void requestJoinFinished(bool success, QString message);

  void groupsReceived(const QJsonObject &data);
  void groupMessageReceived(const QJsonObject &data);
  void groupMembersReceived(const QJsonObject &data);
  void exploreGroupsReceived(const QJsonObject &data);
  void groupRequestsReceived(const QJsonObject &data);
  void requestJoinReceived(const QJsonObject &data);
  void respondToRequestReceived(const QJsonObject &data);

private slots:
  void onGroupsReceived(const QJsonObject &data);
  void onGroupMessageReceived(const QJsonObject &data);
  void onGroupMembersReceived(const QJsonObject &data);
  void onExploreGroupsReceived(const QJsonObject &data);
  void onRequestJoinReceived(const QJsonObject &data);
  void onGroupRequestsReceived(const QJsonObject &data);
  void onRespondToRequestReceived(const QJsonObject &data);

private:
  static GroupHandlers *m_instance;
  QVariantList m_groups;
  QVariantList m_groupMessages;
  QVariantList m_groupMembers;
  QVariantList m_allGroups;
  QVariantList m_groupRequests;
  QString m_exploreMessage;
};

#endif // GROUPHANDLERS_H
