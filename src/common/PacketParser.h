#pragma once
#include <Arduino.h>
#include <vector>
#include "./Utils.h"
#include "./Constants.h"

class PacketParser
{
private:
    struct PacketData
    {
        uint8_t header;             // Заголовок (0xF4 или 0x4F)
        uint8_t length;             // Длина (без учета header и length)
        uint8_t command;            // Команда (с битом ACK если ответ)
        std::vector<uint8_t> bytes; // Данные команды
        uint8_t checksum;           // Контрольная сумма
        bool isValid = false;       // Корректность пакета
        bool isResponse = false;    // Это ответ (ACK/NAK)
        bool isNak = false;         // Это NAK ответ
    };

    PacketData packet;

public:
    struct WithIndex
    {
        uint8_t value;
        explicit WithIndex(uint8_t idx) : value(idx) {}
    };

    struct WithMinLength
    {
        int value;
        explicit WithMinLength(int len) : value(len) {}
    };

    // Базовый парсинг
    bool parseFromString(const String &hexCommand)
    {
        return parseFromString(hexCommand, 0, 0, 0, false, false);
    }

    // Только команда
    bool parseFromString(const String &hexCommand, uint8_t expectedCommand)
    {
        return parseFromString(hexCommand, expectedCommand, 0, 0, true, false);
    }

    // Команда + индекс
    bool parseFromString(const String &hexCommand, uint8_t expectedCommand, WithIndex idx)
    {
        return parseFromString(hexCommand, expectedCommand, idx.value, 0, true, true);
    }

    // Команда + минимальная длина
    bool parseFromString(const String &hexCommand, uint8_t expectedCommand, WithMinLength minLen)
    {
        return parseFromString(hexCommand, expectedCommand, 0, minLen.value, true, false);
    }

    // Команда + индекс + минимальная длина
    bool parseFromString(const String &hexCommand, uint8_t expectedCommand, WithIndex idx, WithMinLength minLen)
    {
        return parseFromString(hexCommand, expectedCommand, idx.value, minLen.value, true, true);
    }

    // ==============================

    // Базовый парсинг
    bool parseFromBytes(const uint8_t *data, size_t length)
    {
        return parseFromBytes(data, length, 0, 0, 0, false, false);
    }

    // Только команда
    bool parseFromBytes(const uint8_t *data, size_t length, uint8_t expectedCommand)
    {
        return parseFromBytes(data, length, expectedCommand, 0, 0, true, false);
    }

    // Команда + индекс
    bool parseFromBytes(const uint8_t *data, size_t length, uint8_t expectedCommand, WithIndex idx)
    {
        return parseFromBytes(data, length, expectedCommand, idx.value, 0, true, true);
    }

    // Команда + минимальная длина
    bool parseFromBytes(const uint8_t *data, size_t length, uint8_t expectedCommand, WithMinLength minLen)
    {
        return parseFromBytes(data, length, expectedCommand, 0, minLen.value, true, false);
    }

    // Команда + индекс + минимальная длина
    bool parseFromBytes(const uint8_t *data, size_t length, uint8_t expectedCommand, WithIndex idx, WithMinLength minLen)
    {
        return parseFromBytes(data, length, expectedCommand, idx.value, minLen.value, true, true);
    }

    uint8_t getHeader() const { return packet.header; }
    uint8_t getLength() const { return packet.length; }
    uint8_t getCommand() const { return packet.command; }
    uint8_t getByteCounts() const { return packet.bytes.size(); }
    uint8_t getCommandWithoutAck() const { return packet.command & 0x7F; }
    const std::vector<uint8_t> &getBytes() const { return packet.bytes; }
    uint8_t *getData()
    {
        return packet.bytes.empty() ? nullptr : packet.bytes.data();
    }
    uint8_t getChecksum() const { return packet.checksum; }
    bool isValid() const { return packet.isValid; }
    bool isResponse() const { return packet.isResponse; }
    bool isNak() const { return packet.isNak; }
    bool isAck() const { return packet.isResponse && !packet.isNak; }

private:
    bool parseFromString(const String &hexCommand, uint8_t expectedCommand, uint8_t expectedIndex, int minLength, bool checkCommand = true, bool checkIndex = true)
    {
        String cleanCmd = hexCommand;
        cleanCmd.replace(" ", "");
        cleanCmd.toLowerCase();

        if (hexCommand.isEmpty() || !Utils::isHexString(cleanCmd) || cleanCmd.length() < 8)
        {
            return false;
        }

        size_t length = cleanCmd.length() / 2;
        uint8_t data[length];
        int byteCount;

        for (size_t i = 0; i < cleanCmd.length(); i += 2)
        {
            data[byteCount++] = Utils::hexStringToByte(cleanCmd.substring(i, i + 2));
        }

        return parseFromBytes(data, byteCount, expectedCommand, expectedIndex, minLength, checkCommand, checkIndex);
    }

    bool parseFromBytes(const uint8_t *data, size_t length, uint8_t expectedCommand, uint8_t expectedIndex, int minLength, bool checkCommand = true, bool checkIndex = true)
    {
        reset();

        if (data == nullptr || length < 4 || length != data[1] + 2)
        {
            if (length < 4)
            {
                Serial.println("Слишком короткий пакет, минимальная длинна – 4 байта");
            }
            else if (length != data[1] + 2)
            {
                Serial.println("Неверная длинна данных, ожидалась: " + String(data[1]) + " | получена: " + String(length - 2));
            }

            return false;
        }

        if (minLength > 0 && length < static_cast<size_t>(minLength))
        {
            Serial.println("Неверная длинна пакета, ожидалась: " + String(minLength) + " | получена: " + String(length));
            return false;
        }

        packet.bytes.assign(data, data + length);

        packet.header = data[0];
        packet.length = data[1];
        packet.command = data[2];
        packet.isResponse = packet.header == RXHEADER;

        if (packet.isResponse && length >= 6)
        {
            packet.isNak = (data[2] == 0x7F);
        }

        packet.checksum = data[length - 1];

        packet.isValid = Utils::validateChecksum(data, length);

        if (!packet.isValid)
        {
            Serial.println("Неверная контрольная сумма, ожидалась: " + Utils::byteToHexString(Utils::calculateChecksum(data, length - 1)) + " | получена: " + Utils::byteToHexString(packet.checksum));
            return false;
        }

        // =========================================================================
        // ВАЛИДАЦИЯ ПАРАМЕТРОВ (если указаны)
        // =========================================================================

        if (checkCommand)
        {
            uint8_t actualCommand = packet.command;
            if (packet.isResponse && !packet.isNak)
            {
                // Для ACK ответов сравниваем без бита ACK
                actualCommand = packet.command & 0x7F;
            }

            if (actualCommand != expectedCommand)
            {
                Serial.println("Неверная ACK бит, ожидалась: " + Utils::byteToHexString(expectedCommand) + " | получена: " + Utils::byteToHexString(actualCommand));
                return false;
            }
        }

        if (checkIndex && (packet.bytes.size() < 5 || data[3] != expectedIndex))
        {
            Serial.println("Неверная индекс, ожидалась: " + Utils::byteToHexString(expectedIndex) + " | получена: " + Utils::byteToHexString(data[3]));
            return false;
        }

        return true;
    }

    void reset()
    {
        packet = PacketData();
    }
};