#include "filesharinghandlers.h"
#include "winsockclient.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QUrl>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

FileSharingHandlers *FileSharingHandlers::m_instance = nullptr;
static const int CHUNK_SIZE = 64 * 1024; // 64KB

// Helper: Convert QString unicode path to std::filesystem::path
static fs::path q2path(const QString &s) { return fs::path(s.toStdWString()); }

FileSharingHandlers *FileSharingHandlers::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new FileSharingHandlers();
  }
  return m_instance;
}

FileSharingHandlers::FileSharingHandlers(QObject *parent) : QObject(parent) {
  connect(this, &FileSharingHandlers::listFilesReceived, this,
          &FileSharingHandlers::onListFilesReceived);
  connect(this, &FileSharingHandlers::uploadChunkAck, this,
          &FileSharingHandlers::onUploadChunkAck);
  connect(this, &FileSharingHandlers::downloadChunkReceived, this,
          &FileSharingHandlers::onDownloadChunkReceived);

  connect(this, &FileSharingHandlers::operationResultReceived, this,
          &FileSharingHandlers::onOperationResultReceived);
  connect(this, &FileSharingHandlers::browseDirectoriesReceived, this,
          &FileSharingHandlers::onBrowseDirectoriesReceived);
}

void FileSharingHandlers::listFiles(int groupID, const QString &path) {
  QJsonObject request;
  request["action"] = "listFiles";
  request["groupID"] = groupID;
  request["path"] = path;
  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::createFolder(int groupID, const QString &path,
                                       const QString &folderName) {
  QJsonObject request;
  request["action"] = "createFolder";
  request["groupID"] = groupID;
  request["path"] = path;
  request["folderName"] = folderName;
  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::uploadFile(int groupID, const QString &path,
                                     const QString &fileUrl) {
  if (m_upload.active) {
    emit uploadFinished(false, "Another upload is in progress.");
    return;
  }

  // Convert QML URL to local path (QString) then to fs::path
  QString localPath = QUrl(fileUrl).toLocalFile();
  fs::path filePath = q2path(localPath);

  // Use std::ifstream
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    emit uploadFinished(false, "Cannot open file (std::ifstream).");
    return;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (file.read(buffer.data(), size)) {
    // Read success
  }
  file.close();

  // Convert to QByteArray for easier splitting/sending with current logic
  // (We could keep it as vector and slice it, but QByteArray is convenient for
  // Base64)
  QByteArray data(buffer.data(), (int)size);

  m_upload.groupID = groupID;
  m_upload.path = path;
  m_upload.fileName =
      QFileInfo(localPath)
          .fileName(); // Can use fs::path::filename but QFileInfo handles
                       // unicode logic well for QString
  m_upload.data = data;
  m_upload.totalChunks = (data.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
  m_upload.currentChunk = 0;
  m_upload.active = true;

  m_uploadProgress = 0.0;
  emit uploadProgressChanged();

  sendNextUploadChunk();
}

void FileSharingHandlers::sendNextUploadChunk() {
  if (!m_upload.active)
    return;

  if (m_upload.currentChunk >= m_upload.totalChunks) {
    m_upload.active = false;
    m_upload.data.clear();
    emit uploadFinished(true, "Upload completed.");
    listFiles(m_upload.groupID, m_upload.path); // Refresh list
    return;
  }

  int start = m_upload.currentChunk * CHUNK_SIZE;
  // min of chunk size or remaining
  int length = CHUNK_SIZE;
  if (start + length > m_upload.data.size()) {
    length = m_upload.data.size() - start;
  }

  QByteArray chunk = m_upload.data.mid(start, length);

  QJsonObject request;
  request["action"] = "uploadChunk";
  request["groupID"] = m_upload.groupID;
  request["path"] = m_upload.path;
  request["fileName"] = m_upload.fileName;
  request["chunkIndex"] = m_upload.currentChunk;
  request["totalChunks"] = m_upload.totalChunks;
  request["data"] = QString(chunk.toBase64());

  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::downloadFile(int groupID, const QString &path,
                                       const QString &fileName,
                                       const QString &saveUrl) {
  if (m_download.active) {
    emit downloadFinished(false, "Another download is in progress.");
    return;
  }

  QString savePath = QUrl(saveUrl).toLocalFile();
  // If it's a directory, append filename
  // Use std::filesystem to check dir
  fs::path fbPath = q2path(savePath);

  if (fs::is_directory(fbPath)) {
    savePath += "/" + fileName;
    fbPath = q2path(savePath);
  }

  m_download.groupID = groupID;
  m_download.path = path;
  m_download.fileName = fileName;
  m_download.savePath = savePath;
  m_download.currentChunk = 0;
  m_download.totalBytesReceived = 0;
  m_download.totalSize = 0;
  m_download.active = true;

  // Cleanup old file ptr if any (should stay null usually)
  if (m_download.file) {
    delete m_download.file;
    m_download.file = nullptr;
  }

  m_download.file = new std::ofstream(fbPath, std::ios::binary | std::ios::out);

  if (!m_download.file->is_open()) {
    m_download.active = false;
    delete m_download.file;
    m_download.file = nullptr;
    emit downloadFinished(false, "Cannot open save file (std::ofstream).");
    return;
  }

  m_downloadProgress = 0.0;
  emit downloadProgressChanged();

  requestNextDownloadChunk();
}

void FileSharingHandlers::requestNextDownloadChunk() {
  if (!m_download.active)
    return;

  QJsonObject request;
  request["action"] = "downloadChunk";
  request["groupID"] = m_download.groupID;
  request["path"] = m_download.path;
  request["fileName"] = m_download.fileName;
  request["chunkIndex"] = m_download.currentChunk;
  request["chunkSize"] = CHUNK_SIZE;

  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::deleteItem(int groupID, const QString &path,
                                     const QString &name, bool isDir) {
  QJsonObject request;
  request["action"] = "deleteItem";
  request["groupID"] = groupID;
  request["userID"] = WinSockClient::getInstance()->getUserId();
  request["path"] = path;
  request["name"] = name;
  request["isDir"] = isDir;
  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::renameItem(int groupID, const QString &path,
                                     const QString &oldName,
                                     const QString &newName) {
  QJsonObject request;
  request["action"] = "renameItem";
  request["groupID"] = groupID;
  request["userID"] = WinSockClient::getInstance()->getUserId();
  request["path"] = path;
  request["oldName"] = oldName;
  request["newName"] = newName;
  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::onListFilesReceived(const QJsonObject &data) {
  if (data["success"].toBool()) {
    m_currentPath = data["currentPath"].toString();
    emit currentPathChanged();

    m_files.clear();
    QJsonArray files = data["files"].toArray();
    for (const auto &val : files) {
      QJsonObject f = val.toObject();
      QVariantMap map;
      map["name"] = f["name"].toString();
      map["isDir"] = f["isDir"].toBool();
      map["size"] = f["size"].toVariant().toLongLong();
      map["date"] = f["date"].toString();
      m_files.append(map);
    }
    emit filesChanged();
  } else {
    qDebug() << "List files error:" << data["message"].toString();
  }
}

void FileSharingHandlers::onUploadChunkAck(const QJsonObject &data) {
  if (!m_upload.active)
    return;

  if (data["success"].toBool()) {
    int ackIndex = data["chunkIndex"].toInt();
    if (ackIndex == m_upload.currentChunk) {
      m_upload.currentChunk++;
      m_uploadProgress = (double)m_upload.currentChunk / m_upload.totalChunks;
      emit uploadProgressChanged();
      sendNextUploadChunk();
    }
  } else {
    m_upload.active = false;
    m_upload.data.clear();
    emit uploadFinished(false, "Upload failed: " + data["message"].toString());
  }
}

void FileSharingHandlers::onDownloadChunkReceived(const QJsonObject &data) {
  if (!m_download.active)
    return;

  if (data["success"].toBool()) {
    int index = data["chunkIndex"].toInt();
    if (index == m_download.currentChunk) {
      QByteArray chunk =
          QByteArray::fromBase64(data["data"].toString().toLatin1());

      // Write using std::ofstream
      if (m_download.file) {
        m_download.file->write(chunk.constData(), chunk.size());
      }

      m_download.totalBytesReceived += chunk.size();

      if (data.contains("totalSize")) {
        m_download.totalSize = data["totalSize"].toVariant().toLongLong();
      }

      if (m_download.totalSize > 0) {
        m_downloadProgress =
            (double)m_download.totalBytesReceived / m_download.totalSize;
        emit downloadProgressChanged();
      }

      if (data["eof"].toBool()) {
        if (m_download.file) {
          m_download.file->close();
          delete m_download.file;
          m_download.file = nullptr;
        }
        m_download.active = false;
        emit downloadFinished(true, "Download completed.");
      } else {
        m_download.currentChunk++;
        requestNextDownloadChunk();
      }
    }
  } else {
    m_download.active = false;
    if (m_download.file) {
      m_download.file->close();
      delete m_download.file;
      m_download.file = nullptr;
    }
    emit downloadFinished(false,
                          "Download failed: " + data["message"].toString());
  }
}

QString FileSharingHandlers::clipboardAction() const {
  return m_clipboardAction;
}

QString FileSharingHandlers::clipboardPath() const { return m_clipboardPath; }

void FileSharingHandlers::setClipboard(const QString &action,
                                       const QString &path) {
  if (m_clipboardAction != action || m_clipboardPath != path) {
    m_clipboardAction = action;
    m_clipboardPath = path;
    emit clipboardActionChanged();
    emit clipboardPathChanged();
  }
}

void FileSharingHandlers::clearClipboard() { setClipboard("", ""); }

void FileSharingHandlers::copyItem(int groupID, const QString &srcPath,
                                   const QString &destPath) {
  QJsonObject request;
  request["action"] = "copyItem";
  request["groupID"] = groupID;
  request["userID"] = WinSockClient::getInstance()->getUserId();
  request["srcPath"] = srcPath;
  request["destPath"] = destPath;
  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::moveItem(int groupID, const QString &srcPath,
                                   const QString &destPath) {
  QJsonObject request;
  request["action"] = "moveItem";
  request["groupID"] = groupID;
  request["userID"] = WinSockClient::getInstance()->getUserId();
  request["srcPath"] = srcPath;
  request["destPath"] = destPath;
  WinSockClient::getInstance()->sendMessage(request);
}

void FileSharingHandlers::onOperationResultReceived(const QJsonObject &data) {
  QString action = data["action"].toString();
  if (data["success"].toBool()) {
    if (action == "createFolder" || action == "deleteItem" ||
        action == "renameItem" || action == "moveItem" ||
        action == "copyItem") {
      qDebug() << "Operation success:" << action;
      if (action == "moveItem" && m_clipboardAction == "move") {
        clearClipboard();
      }
      emit fileOperationFinished(true, "Operation successful.");
    }
  } else {
    qDebug() << "Operation failed:" << action << ":"
             << data["message"].toString();
    emit fileOperationFinished(false, data["message"].toString());
  }
}

QVariantList FileSharingHandlers::files() const { return m_files; }

QString FileSharingHandlers::currentPath() const { return m_currentPath; }

double FileSharingHandlers::uploadProgress() const { return m_uploadProgress; }

double FileSharingHandlers::downloadProgress() const {
  return m_downloadProgress;
}

void FileSharingHandlers::browseDirectories(int groupID, const QString &path) {
  QJsonObject request;
  request["action"] = "browseDirectories";
  request["groupID"] = groupID;
  request["path"] = path;
  WinSockClient::getInstance()->sendMessage(request);
}

QVariantList FileSharingHandlers::moveDialogFolders() const {
  return m_moveDialogFolders;
}

void FileSharingHandlers::onBrowseDirectoriesReceived(const QJsonObject &data) {
  if (data["success"].toBool()) {
    m_moveDialogFolders.clear();
    QJsonArray folders = data["folders"].toArray();
    for (const auto &val : folders) {
      QJsonObject f = val.toObject();
      QVariantMap map;
      map["name"] = f["name"].toString();
      map["path"] = f["path"].toString();
      m_moveDialogFolders.append(map);
    }
    emit moveDialogFoldersChanged();
  } else {
    qDebug() << "Browse directories error:" << data["message"].toString();
  }
}
