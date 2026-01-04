#include "grouphandlers.h"
#include "header.h"
#include "winsockclient.h"
#include <QDebug>
#include <QJsonArray>


GroupHandlers *GroupHandlers::m_instance = nullptr;

GroupHandlers *GroupHandlers::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new GroupHandlers();
  }
  return m_instance;
}

GroupHandlers::GroupHandlers(QObject *parent) : QObject(parent) {
  // Connect internal signals from WinSockClient (via lambda) to handlers
  connect(this, &GroupHandlers::groupsReceived, this,
          &GroupHandlers::onGroupsReceived);
  connect(this, &GroupHandlers::groupMessageReceived, this,
          &GroupHandlers::onGroupMessageReceived);
  connect(this, &GroupHandlers::groupMembersReceived, this,
          &GroupHandlers::onGroupMembersReceived);
}

void GroupHandlers::fetchGroups() {
  int userId = WinSockClient::getInstance()->getUserId();
  QJsonObject request;
  request["action"] = "getUserGroups";
  request["userID"] = userId;
  WinSockClient::getInstance()->sendMessage(request);
}

void GroupHandlers::createGroup(const QString &groupName) {
  int ownerId = WinSockClient::getInstance()->getUserId();
  QJsonObject request;
  request["action"] = "createGroup";
  request["groupName"] = groupName;
  request["ownerID"] = ownerId;
  WinSockClient::getInstance()->sendMessage(request);
}

void GroupHandlers::addUserToGroup(int groupID, int userID) {
  QJsonObject request;
  request["action"] = "addUserToGroup";
  request["groupID"] = groupID;
  request["userID"] = userID;
  WinSockClient::getInstance()->sendMessage(request);
  // Refresh lists after change
  fetchGroups();
  if (WinSockClient::getInstance()->getGroupId() == groupID) {
    loadGroupMessages(groupID);
    loadGroupMembers(groupID);
  }
}

void GroupHandlers::removeUserFromGroup(int groupID, int userID) {
  QJsonObject request;
  request["action"] = "removeUserFromGroup";
  request["groupID"] = groupID;
  request["userID"] = userID;
  WinSockClient::getInstance()->sendMessage(request);
  // Refresh lists after change
  fetchGroups();
  if (WinSockClient::getInstance()->getGroupId() == groupID) {
    loadGroupMessages(groupID);
    loadGroupMembers(groupID);
  }
}

void GroupHandlers::leaveGroup(int groupID) {
  int userID = WinSockClient::getInstance()->getUserId();
  QJsonObject request;
  request["action"] = "leaveGroup";
  request["groupID"] = groupID;
  request["userID"] = userID;
  WinSockClient::getInstance()->sendMessage(request);
}

void GroupHandlers::sendGroupMessage(const QString &content) {
  int senderID = WinSockClient::getInstance()->getUserId();
  int groupID = WinSockClient::getInstance()->getGroupId();
  if (senderID == 0 || groupID == 0 || content.isEmpty()) {
    qWarning() << "sendGroupMessage aborted: groupID=" << groupID
               << ", content empty=" << content.isEmpty();
    clientLogMessage(std::string("[GROUP_SEND_ABORT] userID=") +
                     std::to_string(senderID) +
                     ", groupID=" + std::to_string(groupID) +
                     (content.isEmpty() ? " empty" : " ok"));
    return;
  }

  QJsonObject request;
  request["action"] = "sendGroupMessage";
  request["groupID"] = groupID;
  request["senderID"] = senderID;
  request["content"] = content;
  WinSockClient::getInstance()->sendMessage(request);

  // Trace outgoing group send
  clientLogMessage(std::string("[GROUP_SEND] groupID=") +
                   std::to_string(groupID) +
                   ", senderID=" + std::to_string(senderID) +
                   ", content=" + content.toStdString());

  // Optimistic update
  QVariantMap map;
  map["senderID"] = senderID;
  map["groupID"] = groupID;
  map["content"] = content;
  m_groupMessages.append(map);
  emit groupMessagesChanged();
}

void GroupHandlers::loadGroupMessages(int groupID) {
  QJsonObject request;
  request["action"] = "getGroupMessages";
  request["groupID"] = groupID;
  WinSockClient::getInstance()->sendMessage(request);
}

void GroupHandlers::loadGroupMembers(int groupID) {
  QJsonObject request;
  request["action"] = "getGroupMembers";
  request["groupID"] = groupID;
  WinSockClient::getInstance()->sendMessage(request);
}

void GroupHandlers::onGroupsReceived(const QJsonObject &data) {
  if (data["success"].toBool()) {
    m_groups.clear();
    QJsonArray groups = data["groups"].toArray();
    for (const auto &val : groups) {
      QJsonObject g = val.toObject();
      QVariantMap gm;
      gm["groupID"] = g["groupID"].toInt();
      gm["groupName"] = g["groupName"].toString();
      m_groups.append(gm);
    }
    emit groupsChanged();
  }
}

void GroupHandlers::onGroupMessageReceived(const QJsonObject &data) {
  QString action = data["action"].toString();
  qDebug() << "[GroupHandlers] onGroupMessageReceived - action:" << action;

  // Group history
  if (action == "getGroupMessages") {
    if (data["success"].toBool()) {
      m_groupMessages.clear();
      QJsonArray msgs = data["messages"].toArray();
      for (const auto &val : msgs) {
        QJsonObject obj = val.toObject();
        QVariantMap map;
        map["senderID"] = obj["senderID"].toInt();
        map["senderName"] = obj["senderName"].toString();
        map["content"] = obj["content"].toString();
        map["sentAt"] = obj["sentAt"].toString();
        m_groupMessages.append(map);
      }
      qDebug() << "[GroupHandlers] Loaded" << m_groupMessages.size()
               << "group messages";
      emit groupMessagesChanged();
    }
  }
  // Own message sent successfully - already handled by optimistic update, just
  // log
  else if (action == "sendGroupMessage") {
    if (data["success"].toBool()) {
      qDebug() << "[GroupHandlers] sendGroupMessage success (already "
                  "optimistically updated)";
    } else {
      qDebug() << "[GroupHandlers] sendGroupMessage failed:"
               << data["message"].toString();
      // TODO: Could remove the optimistic update here on failure
    }
  }
  // Group incoming message from another user (real-time)
  else if (action == "receiveGroupMessage") {
    int currentGroup = WinSockClient::getInstance()->getGroupId();
    int groupID = data["groupID"].toInt();
    int senderID = data["senderID"].toInt();
    int myUserID = WinSockClient::getInstance()->getUserId();

    qDebug() << "[GroupHandlers] receiveGroupMessage - currentGroup:"
             << currentGroup << "msgGroupID:" << groupID
             << "senderID:" << senderID << "myUserID:" << myUserID;

    // Only show if it's the current group AND message is from someone else
    // (our own message is already shown via optimistic update)
    if (groupID == currentGroup && senderID != myUserID) {
      QVariantMap map;
      map["senderID"] = senderID;
      map["senderName"] = data["senderName"].toString();
      map["content"] = data["content"].toString();
      map["sentAt"] = data["sentAt"].toString();
      m_groupMessages.append(map);
      qDebug()
          << "[GroupHandlers] Appended group message from other user, total:"
          << m_groupMessages.size();
      emit groupMessagesChanged();
    }
  }
}

void GroupHandlers::onGroupMembersReceived(const QJsonObject &data) {
  if (data["success"].toBool()) {
    m_groupMembers.clear();
    QJsonArray members = data["members"].toArray();
    for (const auto &val : members) {
      QJsonObject m = val.toObject();
      QVariantMap mm;
      mm["userID"] = m["userID"].toInt();
      mm["username"] = m["username"].toString();
      mm["status"] = m["status"].toInt();
      mm["joinedAt"] = m["joinedAt"].toString();
      m_groupMembers.append(mm);
    }
    emit groupMembersChanged();
  } else {
    qDebug() << "Failed to get group members:" << data["message"].toString();
  }
}

QVariantList GroupHandlers::groups() const { return m_groups; }

QVariantList GroupHandlers::groupMessages() const { return m_groupMessages; }

QVariantList GroupHandlers::groupMembers() const { return m_groupMembers; }
