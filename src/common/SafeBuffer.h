// src/common/SafeBuffer.h
#pragma once
#include <Arduino.h>

template<size_t N>
class SafeBuffer
{
private:
    uint8_t buffer[N];
    size_t used = 0;

public:
    SafeBuffer() { clear(); }

    // Запрет копирования
    SafeBuffer(const SafeBuffer&) = delete;
    SafeBuffer& operator=(const SafeBuffer&) = delete;

    // Разрешаем перемещение
    SafeBuffer(SafeBuffer&& other) noexcept 
        : used(other.used)
    {
        if (this != &other)
        {
            memcpy(buffer, other.buffer, other.used);
            other.used = 0;
        }
    }

    SafeBuffer& operator=(SafeBuffer&& other) noexcept
    {
        if (this != &other)
        {
            used = other.used;
            memcpy(buffer, other.buffer, other.used);
            other.used = 0;
        }
        return *this;
    }

    // Добавление данных с проверкой границ
    bool append(const uint8_t* data, size_t len)
    {
        if (data == nullptr || len == 0 || used + len > N)
        {
            return false;
        }

        memcpy(buffer + used, data, len);
        used += len;
        return true;
    }

    // Добавление одного байта
    bool appendByte(uint8_t byte)
    {
        return append(&byte, 1);
    }

    // Безопасное чтение байта
    bool getByte(size_t index, uint8_t* byte) const
    {
        if (byte == nullptr || index >= used)
        {
            return false;
        }
        *byte = buffer[index];
        return true;
    }

    // Копирование в целевой буфер
    bool copyTo(uint8_t* dest, size_t destSize, size_t* copied = nullptr) const
    {
        if (dest == nullptr || destSize < used)
        {
            if (copied) *copied = 0;
            return false;
        }

        memcpy(dest, buffer, used);
        if (copied) *copied = used;
        return true;
    }
    
    // Копирование из другого SafeBuffer
    bool copyFrom(const SafeBuffer<N>& source)
    {
        return append(source.buffer, source.used);
    }

    // Получение подбуфера (безопасно)
    template<size_t M>
    bool getSubBuffer(size_t start, size_t length, SafeBuffer<M>& result) const
    {
        if (start >= used || start + length > used || length == 0)
        {
            return false;
        }

        result.clear();
        return result.append(&buffer[start], length);
    }

    // Получение HEX строки
    String toHexString() const
    {
        String result;
        if (used == 0) return result;
        
        result.reserve(used * 3); // "FF " для каждого байта
        
        for (size_t i = 0; i < used; i++)
        {
            if (i > 0) result += " ";
            if (buffer[i] < 0x10) result += "0";
            result += String(buffer[i], HEX);
        }
        result.toUpperCase();
        return result;
    }

    // Поиск байта
    bool findByte(uint8_t byte, size_t* position = nullptr) const
    {
        for (size_t i = 0; i < used; i++)
        {
            if (buffer[i] == byte)
            {
                if (position) *position = i;
                return true;
            }
        }
        return false;
    }

    // Сравнение с другим буфером
    template<size_t M>
    bool equals(const SafeBuffer<M>& other) const
    {
        if (used != other.used)
        {
            return false;
        }
        
        return memcmp(buffer, other.buffer, used) == 0;
    }

    // Очистка буфера
    void clear()
    {
        used = 0;
        // Не обязательно обнулять память, но для безопасности
        memset(buffer, 0, sizeof(buffer));
    }

    // Проверка заполненности
    bool isFull() const { return used >= N; }
    bool isEmpty() const { return used == 0; }

    // Получение размеров
    size_t size() const { return used; }
    size_t capacity() const { return N; }
    size_t freeSpace() const { return N - used; }

    // Безопасный доступ к данным (только для чтения)
    const uint8_t* data() const { return buffer; }

    // Итерация по байтам
    template<typename Callback>
    bool forEach(Callback callback) const
    {
        for (size_t i = 0; i < used; i++)
        {
            if (!callback(buffer[i], i))
            {
                return false;
            }
        }
        return true;
    }

    // Подсчет контрольной суммы
    uint8_t calculateChecksum() const
    {
        if (used == 0) return 0;
        
        uint8_t checksum = 0;
        for (size_t i = 0; i < used; i++)
        {
            checksum ^= buffer[i];
        }
        return checksum;
    }

    // Валидация контрольной суммы
    bool validateChecksum() const
    {
        if (used < 2) return false;
        
        uint8_t calculated = 0;
        for (size_t i = 0; i < used - 1; i++)
        {
            calculated ^= buffer[i];
        }
        return calculated == buffer[used - 1];
    }
    
    // Оператор сравнения с массивом
    bool operator==(const uint8_t* otherData) const
    {
        if (otherData == nullptr) return false;
        for (size_t i = 0; i < used; i++)
        {
            if (buffer[i] != otherData[i]) return false;
        }
        return true;
    }
};