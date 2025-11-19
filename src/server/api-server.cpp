#include "api-server.h"
#include <ArduinoJson.h>
#include "wbus/wbus.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-errors.h"

ApiServer apiServer;

#define FS LittleFS

ApiServer::ApiServer() : server(80) {}

void ApiServer::serveStaticFile(String path, String contentType)
{
    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        if (file) {
            server.streamFile(file, contentType);
            file.close();
            Serial.println("✅ Обслужен файл: " + path);
        } else {
            server.send(500, "text/plain", "Error opening file");
            Serial.println("❌ Ошибка открытия файла: " + path);
        }
    } else {
        // Fallback - отдаем HTML из кода если файл не найден
        Serial.println("⚠️  Файл не найден, используется резервный HTML: " + path);
    }
}

void ApiServer::begin()
{
        // Инициализация SPIFFS для хранения HTML файлов
    if (!LittleFS.begin(true)) {
        Serial.println("❌ Ошибка инициализации LittleFS");
        // Можно продолжить с HTML в коде
    } else {
        Serial.println("✅ LittleFS инициализирован");
        
        // Проверяем файлы
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            Serial.println("📄 Файл: " + String(file.name()) + " | Размер: " + String(file.size()));
            file = root.openNextFile();
        }
    }
    
    Serial.println("✅ SPIFFS инициализирован");

        Serial.println("📁 Содержимое LittleFS:");
    File root = FS.open("/");
    if (!root) {
        Serial.println("   ❌ Не удалось открыть корневую директорию");
        return;
    }
    
    if (!root.isDirectory()) {
        Serial.println("   ❌ Корень не является директорией");
        return;
    }
    
    File file = root.openNextFile();
    while (file) {
        Serial.println("   📄 " + String(file.name()) + " | Размер: " + String(file.size()) + " байт");
        file = root.openNextFile();
    }
    root.close();

    // Основные endpoint-ы
    server.on("/", HTTP_GET, [this]() { serveHTML(); });
    
    // Новые endpoint-ы для данных
    server.on("/api/system/state", HTTP_GET, [this]() { handleGetSystemState(); });
    server.on("/api/device/info", HTTP_GET, [this]() { handleGetDeviceInfo(); });
    server.on("/api/sensors/data", HTTP_GET, [this]() { handleGetSensorsData(); });
    server.on("/api/errors", HTTP_GET, [this]() { handleGetErrors(); });
    server.on("/api/all", HTTP_GET, [this]() { handleGetAllData(); });
    server.on("/api/system/state", HTTP_GET, [this]() { handleGetSystemState(); });
    server.on("/api/device/info", HTTP_GET, [this]() { handleGetDeviceInfo(); });
    server.on("/api/sensors/data", HTTP_GET, [this]() { handleGetSensorsData(); });
    server.on("/api/errors", HTTP_GET, [this]() { handleGetErrors(); });
    server.on("/api/all", HTTP_GET, [this]() { handleGetAllData(); });

    // Новые endpoint-ы для управления
    server.on("/api/control/connect", HTTP_POST, [this]() { handleConnect(); });
    server.on("/api/control/disconnect", HTTP_POST, [this]() { handleDisconnect(); });
    server.on("/api/control/start_parking", HTTP_POST, [this]() { handleStartParkingHeat(); });
    server.on("/api/control/stop", HTTP_POST, [this]() { handleStopHeater(); });
    server.on("/api/control/toggle_logging", HTTP_POST, [this]() { handleToggleLogging(); });

    // Endpoint-ы для тестирования компонентов
    server.on("/api/test/combustion_fan", HTTP_POST, [this]() {
        int seconds = server.arg("seconds").toInt();
        int power = server.arg("power").toInt();
        wBus.testCombustionFan(seconds, power);
        server.send(200, "application/json", "{\"status\":\"test_started\"}");
    });
    
    // Добавьте аналогичные endpoint-ы для других тестов...

    server.onNotFound([this]() { handleNotFound(); });
    server.begin();
    Serial.println("HTTP server started on port 80");
    Serial.println("Available endpoints:");
    Serial.println("  GET /api/system/state - System connection and heater state");
    Serial.println("  GET /api/device/info  - Device information");
    Serial.println("  GET /api/sensors/data - Sensors data");
    Serial.println("  GET /api/errors       - Error information");
    Serial.println("  GET /api/all          - All data combined");
}

void ApiServer::serveHTML()
{
    serveStaticFile("/index.html", "text/html");
}

void ApiServer::handleConnect()
{
    wBus.connect();
    server.send(200, "application/json", "{\"status\":\"connecting\"}");
}

void ApiServer::handleDisconnect()
{
    wBus.disconnect();
    server.send(200, "application/json", "{\"status\":\"disconnecting\"}");
}

void ApiServer::handleStartParkingHeat()
{
    wBus.startParkingHeat();
    server.send(200, "application/json", "{\"status\":\"started\"}");
}

void ApiServer::handleStopHeater()
{
    wBus.shutdown();
    server.send(200, "application/json", "{\"status\":\"stopped\"}");
}

void ApiServer::handleToggleLogging()
{
    if (wBus.isLogging()) {
        wBus.stopLogging();
    } else {
        wBus.startLogging();
    }
    server.send(200, "application/json", "{\"status\":\"toggled\"}");
}

void ApiServer::loop()
{
    server.handleClient();
}

// 1. Endpoint для состояния системы
void ApiServer::handleGetSystemState()
{
    DynamicJsonDocument doc(1024);
    
    // Состояние подключения
    String connStates[] = {"DISCONNECTED", "CONNECTING", "CONNECTED", "CONNECTION_FAILED"};
    doc["connection_state"] = connStates[wBus.getConnectionState()];
    doc["connection_state_code"] = wBus.getConnectionState();
    
    // Состояние нагревателя
    doc["heater_state"] = wBus.getCurrentStateName();
    doc["heater_state_code"] = wBus.getState();
    
    // Дополнительная информация
    doc["is_connected"] = wBus.isConnected();
    doc["is_logging"] = wBus.isLogging();
    doc["last_rx_time"] = millis(); // Можно добавить реальное время последнего ответа
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

// 2. Endpoint для информации об устройстве
void ApiServer::handleGetDeviceInfo()
{
    WebastoDeviceInfo info = webastoInfo.getDeviceInfo();
    
    DynamicJsonDocument doc(2048);
    
    doc["type"] = "device_info";
    doc["has_data"] = info.hasData();
    doc["last_update"] = info.lastUpdate;
    
    // Основная информация
    doc["wbus_version"] = info.wbusVersion;
    doc["device_name"] = info.deviceName;
    doc["device_id"] = info.deviceID;
    doc["serial_number"] = info.serialNumber;
    doc["test_stand_code"] = info.testStandCode;
    
    // Даты производства
    doc["controller_manufacture_date"] = info.controllerManufactureDate;
    doc["heater_manufacture_date"] = info.heaterManufactureDate;
    
    // Коды и идентификаторы
    doc["customer_id"] = info.customerID;
    doc["additional_code"] = info.additionalCode;
    doc["wbus_code"] = info.wbusCode;
    doc["supported_functions"] = info.supportedFunctions;
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

// 3. Endpoint для данных сенсоров
void ApiServer::handleGetSensorsData()
{
    DynamicJsonDocument doc(4096);
    
    // Операционные измерения
    OperationalMeasurements op = webastoSensors.getCurrentMeasurements();
    JsonObject operational = doc.createNestedObject("operational_measurements");
    operational["temperature"] = op.temperature;
    operational["voltage"] = op.voltage;
    operational["heating_power"] = op.heatingPower;
    operational["flame_resistance"] = op.flameResistance;
    operational["flame_detected"] = op.flameDetected;
    
    // Настройки топлива
    FuelSettings fuel = webastoSensors.getFuelSettingsData();
    JsonObject fuelSettings = doc.createNestedObject("fuel_settings");
    fuelSettings["fuel_type"] = fuel.fuelType;
    fuelSettings["fuel_type_name"] = fuel.fuelTypeName;
    fuelSettings["max_heating_time"] = fuel.maxHeatingTime;
    fuelSettings["ventilation_factor"] = fuel.ventilationFactor;
    
    // Флаги вкл/выкл
    OnOffFlags onOff = webastoSensors.getOnOffFlagsData();
    JsonObject onOffFlags = doc.createNestedObject("on_off_flags");
    onOffFlags["combustion_air_fan"] = onOff.combustionAirFan;
    onOffFlags["glow_plug"] = onOff.glowPlug;
    onOffFlags["fuel_pump"] = onOff.fuelPump;
    onOffFlags["circulation_pump"] = onOff.circulationPump;
    onOffFlags["vehicle_fan_relay"] = onOff.vehicleFanRelay;
    onOffFlags["nozzle_stock_heating"] = onOff.nozzleStockHeating;
    onOffFlags["flame_indicator"] = onOff.flameIndicator;
    onOffFlags["active_components"] = onOff.activeComponents;
    
    // Статусные флаги
    StatusFlags status = webastoSensors.getStatusFlagsData();
    JsonObject statusFlags = doc.createNestedObject("status_flags");
    statusFlags["main_switch"] = status.mainSwitch;
    statusFlags["supplemental_heat_request"] = status.supplementalHeatRequest;
    statusFlags["parking_heat_request"] = status.parkingHeatRequest;
    statusFlags["ventilation_request"] = status.ventilationRequest;
    statusFlags["summer_mode"] = status.summerMode;
    statusFlags["external_control"] = status.externalControl;
    statusFlags["generator_signal"] = status.generatorSignal;
    statusFlags["boost_mode"] = status.boostMode;
    statusFlags["auxiliary_drive"] = status.auxiliaryDrive;
    statusFlags["ignition_signal"] = status.ignitionSignal;
    statusFlags["status_summary"] = status.statusSummary;
    statusFlags["operation_mode"] = status.operationMode;
    
    // Состояние работы
    OperatingState opState = webastoSensors.getOperatingStateData();
    JsonObject operatingState = doc.createNestedObject("operating_state");
    operatingState["state_code"] = opState.stateCode;
    operatingState["state_number"] = opState.stateNumber;
    operatingState["device_state_flags"] = opState.deviceStateFlags;
    operatingState["state_name"] = opState.stateName;
    operatingState["state_description"] = opState.stateDescription;
    operatingState["device_state_info"] = opState.deviceStateInfo;
    
    // Статус подсистем
    SubsystemsStatus subsystems = webastoSensors.getSubsystemsStatusData();
    JsonObject subsystemsStatus = doc.createNestedObject("subsystems_status");
    subsystemsStatus["glow_plug_power"] = subsystems.glowPlugPower;
    subsystemsStatus["glow_plug_power_percent"] = subsystems.glowPlugPowerPercent;
    subsystemsStatus["fuel_pump_frequency"] = subsystems.fuelPumpFrequency;
    subsystemsStatus["fuel_pump_frequency_hz"] = subsystems.fuelPumpFrequencyHz;
    subsystemsStatus["combustion_fan_power"] = subsystems.combustionFanPower;
    subsystemsStatus["combustion_fan_power_percent"] = subsystems.combustionFanPowerPercent;
    subsystemsStatus["circulation_pump_power"] = subsystems.circulationPumpPower;
    subsystemsStatus["circulation_pump_power_percent"] = subsystems.circulationPumpPowerPercent;
    subsystemsStatus["unknown_byte_3"] = subsystems.unknownByte3;
    subsystemsStatus["status_summary"] = subsystems.statusSummary;
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

// 4. Endpoint для ошибок
void ApiServer::handleGetErrors()
{
    ErrorCollection errors = webastoErrors.getErrors();
    
    DynamicJsonDocument doc(4096);
    
    doc["has_errors"] = errors.hasErrors;
    doc["error_count"] = errors.errorCount;
    doc["last_update"] = errors.lastUpdate;
    
    JsonArray errorArray = doc.createNestedArray("errors");
    
    for (const WebastoError& error : errors.errors) {
        JsonObject errorObj = errorArray.createNestedObject();
        errorObj["code"] = error.code;
        errorObj["hex_code"] = error.hexCode;
        errorObj["description"] = error.description;
        errorObj["counter"] = error.counter;
        errorObj["is_active"] = error.isActive;
    }
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

// 5. Endpoint для всех данных сразу
void ApiServer::handleGetAllData()
{
    DynamicJsonDocument doc(8192);
    
    // Системное состояние
    String connStates[] = {"DISCONNECTED", "CONNECTING", "CONNECTED", "CONNECTION_FAILED"};
    JsonObject systemState = doc.createNestedObject("system_state");
    systemState["connection_state"] = connStates[wBus.getConnectionState()];
    systemState["connection_state_code"] = wBus.getConnectionState();
    systemState["heater_state"] = wBus.getCurrentStateName();
    systemState["heater_state_code"] = wBus.getState();
    systemState["is_connected"] = wBus.isConnected();
    systemState["is_logging"] = wBus.isLogging();
    
    // Информация об устройстве
    WebastoDeviceInfo info = webastoInfo.getDeviceInfo();
    JsonObject deviceInfo = doc.createNestedObject("device_info");
    deviceInfo["has_data"] = info.hasData();
    deviceInfo["last_update"] = info.lastUpdate;
    deviceInfo["wbus_version"] = info.wbusVersion;
    deviceInfo["device_name"] = info.deviceName;
    deviceInfo["device_id"] = info.deviceID;
    deviceInfo["serial_number"] = info.serialNumber;
    deviceInfo["controller_manufacture_date"] = info.controllerManufactureDate;
    deviceInfo["heater_manufacture_date"] = info.heaterManufactureDate;
    deviceInfo["customer_id"] = info.customerID;
    deviceInfo["wbus_code"] = info.wbusCode;
    deviceInfo["supported_functions"] = info.supportedFunctions;
    
    // Данные сенсоров (упрощенная версия для экономии места)
    JsonObject sensors = doc.createNestedObject("sensors_data");
    
    OperationalMeasurements op = webastoSensors.getCurrentMeasurements();
    JsonObject operational = sensors.createNestedObject("operational");
    operational["temperature"] = op.temperature;
    operational["voltage"] = op.voltage;
    operational["heating_power"] = op.heatingPower;
    
    StatusFlags status = webastoSensors.getStatusFlagsData();
    JsonObject statusFlags = sensors.createNestedObject("status");
    statusFlags["operation_mode"] = status.operationMode;
    statusFlags["main_switch"] = status.mainSwitch;
    
    OperatingState opState = webastoSensors.getOperatingStateData();
    JsonObject operatingState = sensors.createNestedObject("operating_state");
    operatingState["state_name"] = opState.stateName;
    operatingState["state_code"] = opState.stateCode;
    
    // Ошибки
    ErrorCollection errors = webastoErrors.getErrors();
    JsonObject errorInfo = doc.createNestedObject("errors");
    errorInfo["has_errors"] = errors.hasErrors;
    errorInfo["error_count"] = errors.errorCount;
    
    JsonArray errorArray = errorInfo.createNestedArray("error_list");
    for (const WebastoError& error : errors.errors) {
        JsonObject errorObj = errorArray.createNestedObject();
        errorObj["code"] = error.code;
        errorObj["description"] = error.description;
        errorObj["counter"] = error.counter;
    }
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

void ApiServer::handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
    server.send(404, "text/plain", message);
}