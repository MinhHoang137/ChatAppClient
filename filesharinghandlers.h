#ifndef FILESHARINGHANDLERS_H
#define FILESHARINGHANDLERS_H

#include <QJsonObject>
#include <QObject>
#include <QQmlEngine>
#include <QVariantList>
#include <fstream>
#include <iostream>
#include <memory>

class FileSharingHandlers : public QObject {
  Q_OBJECT
  QML_ELEMENT
  Q_PROPERTY(QVariantList files READ files NOTIFY filesChanged)
  Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
  Q_PROPERTY(
      double uploadProgress READ uploadProgress NOTIFY uploadProgressChanged)
  Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY
                 downloadProgressChanged)
  Q_PROPERTY(QString clipboardAction READ clipboardAction NOTIFY
                 clipboardActionChanged)
  Q_PROPERTY(
      QString clipboardPath READ clipboardPath NOTIFY clipboardPathChanged)
  Q_PROPERTY(QVariantList moveDialogFolders READ moveDialogFolders NOTIFY
                 moveDialogFoldersChanged)

public:
  static FileSharingHandlers *getInstance();
  explicit FileSharingHandlers(QObject *parent = nullptr);

  Q_INVOKABLE void listFiles(int groupID, const QString &path);
  Q_INVOKABLE void browseDirectories(int groupID, const QString &path);
  Q_INVOKABLE void createFolder(int groupID, const QString &path,
                                const QString &folderName);
  Q_INVOKABLE void uploadFile(int groupID, const QString &path,
                              const QString &fileUrl);
  Q_INVOKABLE void downloadFile(int groupID, const QString &path,
                                const QString &fileName,
                                const QString &saveUrl);
  Q_INVOKABLE void deleteItem(int groupID, const QString &path,
                              const QString &name, bool isDir);
  Q_INVOKABLE void renameItem(int groupID, const QString &path,
                              const QString &oldName, const QString &newName);

  Q_INVOKABLE void copyItem(int groupID, const QString &srcPath,
                            const QString &destPath);
  Q_INVOKABLE void moveItem(int groupID, const QString &srcPath,
                            const QString &destPath);

  // Clipboard helpers
  Q_INVOKABLE void setClipboard(const QString &action, const QString &path);
  Q_INVOKABLE void clearClipboard();
  QString clipboardAction() const;
  QString clipboardPath() const;

  QVariantList files() const;
  QString currentPath() const;
  QVariantList clipboard() const;
  QVariantList moveDialogFolders() const;
  double uploadProgress() const;
  double downloadProgress() const;

signals:
  void filesChanged();
  void currentPathChanged();
  void operationResultReceived(const QJsonObject &data);
  void fileOperationFinished(bool success, QString message);
  void uploadProgressChanged();
  void downloadProgressChanged();
  void uploadFinished(bool success, QString message);
  void downloadFinished(bool success, QString message);

  void clipboardActionChanged();
  void clipboardPathChanged();

  // Internal signals from WinSockClient
  void listFilesReceived(const QJsonObject &data);
  void browseDirectoriesReceived(const QJsonObject &data);
  void uploadChunkAck(const QJsonObject &data);
  void downloadChunkReceived(const QJsonObject &data);
  void clipboardChanged();
  void moveDialogFoldersChanged();

private slots:
  void onListFilesReceived(const QJsonObject &data);
  void onBrowseDirectoriesReceived(const QJsonObject &data);
  void onUploadChunkAck(const QJsonObject &data);
  void onDownloadChunkReceived(const QJsonObject &data);
  void onOperationResultReceived(const QJsonObject &data);

private:
  static FileSharingHandlers *m_instance;
  QVariantList m_files;
  QString m_currentPath;
  QVariantList m_clipboard; // List of objects { "name": "foo.txt", "path":
                            // "docs/", "type": "copy"|"cut" }
  QVariantList m_moveDialogFolders;
  double m_uploadProgress = 0.0;
  double m_downloadProgress = 0.0;

  // Upload State
  struct UploadState {
    int groupID;
    QString path;
    QString fileName;
    QByteArray data;
    int currentChunk = 0;
    int totalChunks = 0;
    bool active = false;
  } m_upload;

  // Download State

  // Forward declare instead of include if possible, but for struct member we
  // need type or ptr We will use std::ofstream*

  struct DownloadState {
    int groupID;
    QString path;
    QString fileName;
    QString savePath;
    std::ofstream *file = nullptr; // Changed from QFile*
    int currentChunk = 0;
    qint64 totalBytesReceived = 0;
    qint64 totalSize = 0;
    bool active = false;
  } m_download;

  void sendNextUploadChunk();
  void requestNextDownloadChunk();
  QString m_clipboardAction;
  QString m_clipboardPath;
};

#endif // FILESHARINGHANDLERS_H
