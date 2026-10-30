
#include <Arduino.h>
#include <NimBLEDevice.h>

// ==========================================
// BLE CONFIGURATION
// ==========================================

#define DEVICE_NAME "ESP32-Sensor"

#define SERVICE_UUID \
  "12345678-1234-1234-1234-123456789001"

#define CHARACTERISTIC_UUID \
  "12345678-1234-1234-1234-123456789002"

// ==========================================
// GLOBAL VARIABLES
// ==========================================

NimBLECharacteristic* sensorCharacteristic = nullptr;

bool deviceConnected = false;

uint32_t sequenceNumber = 0;

const uint32_t TRANSMISSION_INTERVAL = 1000;

uint32_t lastTransmission = 0;

// ==========================================
// BLE SERVER CALLBACKS
// ==========================================

class ServerCallbacks : public NimBLEServerCallbacks {

  void onConnect(
    NimBLEServer* server,
    NimBLEConnInfo& connInfo
  ) override {

    deviceConnected = true;

    Serial.println("Smartphone connected!");

  }

  void onDisconnect(
    NimBLEServer* server,
    NimBLEConnInfo& connInfo,
    int reason
  ) override {

    deviceConnected = false;

    Serial.println("Smartphone disconnected!");

    // Restart advertising
    NimBLEDevice::startAdvertising();

  }

};

// ==========================================
// SENSOR ACQUISITION
// ==========================================

// Replace with actual sensor readings.

float readTemperature() {

  return 23.5;

}

float readHumidity() {

  return 55.0;

}

// ==========================================
// BLE INITIALIZATION
// ==========================================

void initializeBLE() {

  NimBLEDevice::init(DEVICE_NAME);

  // Request a larger BLE MTU.
  NimBLEDevice::setMTU(185);

  NimBLEServer* server =
    NimBLEDevice::createServer();

  server->setCallbacks(new ServerCallbacks());

  // Create BLE service
  NimBLEService* service =
    server->createService(SERVICE_UUID);

  // Create notification characteristic
  sensorCharacteristic =
    service->createCharacteristic(
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::NOTIFY
    );

  // Start service
  service->start();

  // Configure advertising
  NimBLEAdvertising* advertising =
    NimBLEDevice::getAdvertising();

  advertising->addServiceUUID(SERVICE_UUID);

  advertising->start();

  Serial.println("BLE initialized");
  Serial.println("Waiting for smartphone...");

}

// ==========================================
// SEND SENSOR DATA
// ==========================================

void transmitMeasurements() {

  float temperature = readTemperature();

  float humidity = readHumidity();

  uint32_t uptime = millis();

  char payload[180];

  snprintf(
    payload,
    sizeof(payload),

    "{\"id\":\"esp32-001\","
    "\"seq\":%lu,"
    "\"temp\":%.2f,"
    "\"humidity\":%.2f,"
    "\"uptime_ms\":%lu}",

    (unsigned long)sequenceNumber++,
    temperature,
    humidity,
    (unsigned long)uptime
  );

  // Update characteristic value
  sensorCharacteristic->setValue(
    (uint8_t*)payload,
    strlen(payload)
  );

  // Notify smartphone
  sensorCharacteristic->notify();

  Serial.print("Sent: ");
  Serial.println(payload);

}

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("ESP32 Mobile Gateway Sensor");
  Serial.println("===========================");

  initializeBLE();

}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  uint32_t now = millis();

  if (
    deviceConnected &&
    now - lastTransmission >= TRANSMISSION_INTERVAL
  ) {

    lastTransmission = now;

    transmitMeasurements();

  }

  delay(10);

}
