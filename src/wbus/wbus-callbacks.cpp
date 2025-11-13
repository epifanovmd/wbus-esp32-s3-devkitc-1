#include "wbus/wbus-callbacks.h"
#include "wbus/wbus-decoders.h"
#include "wbus/wbus-queue.h"

// Глобальные переменные для хранения данных
struct DeviceInfo {
    String deviceName = "N/A";
    String wbusVersion = "N/A";
    String wbusCode = "N/A";
    String deviceId = "N/A";
    String hwVersion = "N/A";
    OperationalMeasurements measurements = {};
    SensorStatusFlags statusFlags = {};
    OnOffFlags onOffFlags = {};
    OperatingTimes operatingTimes = {};
    SubsystemsStatus subsystems = {};
    String operatingState = "N/A";
    String errors = "N/A";
};

DeviceInfo deviceInfo;
int infoCollectionStep = 0;

void collectFullDeviceInfo() {
    infoCollectionStep = 0;
    deviceInfo = DeviceInfo(); // Сброс данных
    
    Serial.println("🔄 Начинаем сбор информации об устройстве...");
    
    // Последовательность запросов
    wbusQueue.add(CMD_READ_INFO_DEVICE_NAME, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_INFO_WBUS_VERSION, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_INFO_WBUS_CODE, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_INFO_DEVICE_ID, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_INFO_HW_VERSION, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_SENSOR_OPERATIONAL, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_SENSOR_STATUS_FLAGS, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_SENSOR_ON_OFF_FLAGS, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_SENSOR_OPERATING_TIMES, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_SENSOR_SUBSYSTEMS_STATUS, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_SENSOR_OPERATING_STATE, deviceInfoCollectorCallback);
    wbusQueue.add(CMD_READ_ERRORS_LIST, deviceInfoCollectorCallback);
}

void deviceInfoCollectorCallback(bool success, String cmd, String response) {
    infoCollectionStep++;
    
    if (success) {
        if (cmd == CMD_READ_INFO_DEVICE_NAME) {
            deviceInfo.deviceName = WBusDecoders::decodeRawResponse(response);
        }
        else if (cmd == CMD_READ_INFO_WBUS_VERSION) {
            if (response.length() >= 8) {
                deviceInfo.wbusVersion = WBusDecoders::decodeWbusVersion(
                    WBusDecoders::hexStringToByte(response.substring(6, 8))
                );
            }
        }
        else if (cmd == CMD_READ_INFO_WBUS_CODE) {
            deviceInfo.wbusCode = WBusDecoders::decodeWbusCode(response);
        }
        else if (cmd == CMD_READ_INFO_DEVICE_ID) {
            deviceInfo.deviceId = WBusDecoders::decodeRawResponse(response);
        }
        else if (cmd == CMD_READ_INFO_HW_VERSION) {
            deviceInfo.hwVersion = WBusDecoders::decodeRawResponse(response);
        }
        else if (cmd == CMD_READ_SENSOR_OPERATIONAL) {
            deviceInfo.measurements = WBusDecoders::decodeOperationalMeasurements(response);
        }
        else if (cmd == CMD_READ_SENSOR_STATUS_FLAGS) {
            deviceInfo.statusFlags = WBusDecoders::decodeStatusFlags(response);
        }
        else if (cmd == CMD_READ_SENSOR_ON_OFF_FLAGS) {
            deviceInfo.onOffFlags = WBusDecoders::decodeOnOffFlags(response);
        }
        else if (cmd == CMD_READ_SENSOR_OPERATING_TIMES) {
            deviceInfo.operatingTimes = WBusDecoders::decodeOperatingTimes(response);
        }
        else if (cmd == CMD_READ_SENSOR_SUBSYSTEMS_STATUS) {
            deviceInfo.subsystems = WBusDecoders::decodeSubsystemsStatus(response);
        }
        else if (cmd == CMD_READ_SENSOR_OPERATING_STATE) {
            if (response.length() >= 8) {
                byte state = WBusDecoders::hexStringToByte(response.substring(6, 8));
                deviceInfo.operatingState = WBusDecoders::decodeOperatingState(state);
            }
        }
        else if (cmd == CMD_READ_ERRORS_LIST) {
            deviceInfo.errors = (response.length() > 10) ? "Есть ошибки" : "Нет ошибок";
        }
        
        Serial.print("✅ Шаг ");
        Serial.print(infoCollectionStep);
        Serial.println(" выполнен");
        
        // Если это последний шаг - выводим сводку
        if (infoCollectionStep >= 11) {
            printDeviceSummary();
        }
    } else {
        Serial.print("❌ Ошибка на шаге ");
        Serial.println(infoCollectionStep);
    }
}

void printDeviceSummary() {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════════════════╗");
    Serial.println("║                 📊 ПОЛНАЯ ИНФОРМАЦИЯ                  ║");
    Serial.println("╠══════════════════════════════════════════════════════╣");
    
    // Основная информация
    Serial.println("║                   🏷️  ОСНОВНАЯ ИНФОРМАЦИЯ             ║");
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.print("║ Имя устройства: "); Serial.print(deviceInfo.deviceName); 
    Serial.println("                  ║");
    Serial.print("║ ID устройства: "); Serial.print(deviceInfo.deviceId);
    Serial.println("                     ║");
    Serial.print("║ Версия W-Bus: "); Serial.print(deviceInfo.wbusVersion);
    Serial.println("                              ║");
    Serial.print("║ Версия железа: "); Serial.print(deviceInfo.hwVersion);
    Serial.println("                            ║");
    Serial.print("║ WBUS-код: "); Serial.print(deviceInfo.wbusCode.substring(0, 30));
    Serial.println(" ║");
    
    // Измерения
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.println("║                    📈 ИЗМЕРЕНИЯ                       ║");
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.printf("║ Температура: %6.1f °C", deviceInfo.measurements.temperature);
    Serial.println("                         ║");
    Serial.printf("║ Напряжение:  %6.1f V", deviceInfo.measurements.voltage);
    Serial.println("                          ║");
    Serial.printf("║ Мощность:    %6d W", deviceInfo.measurements.heatingPower);
    Serial.println("                          ║");
    Serial.printf("║ Пламя:       %14s", 
                  deviceInfo.measurements.flameDetected ? "Обнаружено" : "Нет");
    Serial.println("              ║");
    
    // Статус
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.println("║                    🔧 СТАТУС                          ║");
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.print("║ Состояние: "); Serial.print(deviceInfo.operatingState);
    Serial.println("                ║");
    Serial.print("║ Ошибки: "); Serial.print(deviceInfo.errors);
    Serial.println("                                ║");
    
    // Время работы
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.println("║                    ⏱️  ВРЕМЯ РАБОТЫ                    ║");
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.printf("║ Часы работы: %4d ч %2d мин", 
                  deviceInfo.operatingTimes.workingHours,
                  deviceInfo.operatingTimes.workingMinutes);
    Serial.println("               ║");
    Serial.printf("║ Часы работы (опер): %4d ч %2d мин",
                  deviceInfo.operatingTimes.operatingHours,
                  deviceInfo.operatingTimes.operatingMinutes);
    Serial.println("        ║");
    Serial.printf("║ Счетчик запусков: %6d", deviceInfo.operatingTimes.startCounter);
    Serial.println("                    ║");
    
    // Подсистемы
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.println("║                  🔌 ПОДСИСТЕМЫ                       ║");
    Serial.println("╠══════════════════════════════════════════════════════╣");
    Serial.printf("║ ТЭН: %3d%%  Насос: %3d%%  Вентилятор: %3d%%",
                  deviceInfo.subsystems.glowPlugPower,
                  deviceInfo.subsystems.circulationPumpPower, 
                  deviceInfo.subsystems.combustionFanPower);
    Serial.println("   ║");
    
    Serial.println("╚══════════════════════════════════════════════════════╝");
    Serial.println();
}