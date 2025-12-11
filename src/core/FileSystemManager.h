#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "./common/Utils.h"

class FileSystemManager
{
private:
    bool initialized = false;

public:
    FileSystemManager() {}

    ~FileSystemManager()
    {
        end();
    }

    bool begin(bool formatOnFail = true)
    {
        if (initialized)
        {
            return true;
        }

        initialized = LittleFS.begin(formatOnFail);

        if (initialized)
        {
            Serial.println("✅ LittleFS initialized successfully");
            // printFileSystemInfo();
        }
        else
        {
            Serial.println("❌ LittleFS initialization failed");
        }

        return initialized;
    }

    bool isInitialized() const
    {
        return initialized;
    }

    File open(const char *path, const char *mode)
    {
        if (!initialized && !begin())
        {
            Serial.println("❌ Cannot open file: filesystem not initialized");
            return File();
        }
        return LittleFS.open(path, mode);
    }

    File open(const String &path, const char *mode)
    {
        return open(path.c_str(), mode);
    }

    bool exists(const char *path)
    {
        if (!initialized && !begin())
        {
            return false;
        }
        return LittleFS.exists(path);
    }


    bool exists(const String &path)
    {
        return exists(path.c_str());
    }

    bool remove(const char *path)
    {
        if (!initialized && !begin())
        {
            return false;
        }
        return LittleFS.remove(path);
    }

    bool rename(const char *pathFrom, const char *pathTo)
    {
        if (!initialized && !begin())
        {
            return false;
        }
        return LittleFS.rename(pathFrom, pathTo);
    }

    bool mkdir(const char *path)
    {
        if (!initialized && !begin())
        {
            return false;
        }
        return LittleFS.mkdir(path);
    }

    bool rmdir(const char *path)
    {
        if (!initialized && !begin())
        {
            return false;
        }
        return LittleFS.rmdir(path);
    }

    void getInfo(size_t &totalBytes, size_t &usedBytes)
    {
        if (initialized)
        {
            totalBytes = LittleFS.totalBytes();
            usedBytes = LittleFS.usedBytes();
        }
        else
        {
            totalBytes = 0;
            usedBytes = 0;
        }
    }

    void end()
    {
        if (initialized)
        {
            LittleFS.end();
            initialized = false;
            Serial.println("🔌 LittleFS unmounted");
        }
    }

    bool restart(bool formatOnFail = true)
    {
        end();
        return begin(formatOnFail);
    }

private:
    void printFileSystemInfo()
    {
        size_t totalBytes = LittleFS.totalBytes();
        size_t usedBytes = LittleFS.usedBytes();
        size_t freeBytes = totalBytes - usedBytes;
        float usedPercent = totalBytes > 0 ? (usedBytes * 100.0f) / totalBytes : 0;

        Serial.println("📁 File System Information:");
        Serial.printf("  Total: %s\n", Utils::formatSizeBytes(totalBytes).c_str());
        Serial.printf("  Used:  %s (%.1f%%)\n", Utils::formatSizeBytes(usedBytes).c_str(), usedPercent);
        Serial.printf("  Free:  %s\n", Utils::formatSizeBytes(freeBytes).c_str());

        // Перечисляем корневые файлы
        Serial.println("  Root files:");
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        int fileCount = 0;

        while (file)
        {
            String fileName = file.name();
            size_t fileSize = file.size();
            Serial.printf("    %-20s %s\n", fileName.c_str(), Utils::formatSizeBytes(fileSize).c_str());
            file = root.openNextFile();
            fileCount++;
        }

        if (fileCount == 0)
        {
            Serial.println("    (empty)");
        }
        root.close();
    }
};