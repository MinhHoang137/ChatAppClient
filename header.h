#ifndef HEADER_H
#define HEADER_H

#include <string>

// Khởi tạo file log phía Client (tạo thư mục logs/ và file theo timestamp)
void initClientLog();

// Ghi một dòng log vào file log của Client
void clientLogMessage(const std::string &message);

#endif // HEADER_H
