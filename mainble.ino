#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_LSM9DS1.h>
// Modified from Science Kit
const int VERS = 0x1;
#define ARD_UUID(val) ("5a5a0002-" val "-467a-9538-01f0652c74e8")

BLEService service(ARD_UUID("0000"));
BLEUnsignedIntCharacteristic versionCharacteristic(ARD_UUID("0000"), BLERead);
BLECharacteristic accelCharacteristic(ARD_UUID("0001"), BLENotify,
                                      3 * sizeof(float));
BLECharacteristic gyroCharacteristic(ARD_UUID("0011"), BLENotify,
                                     3 * sizeof(float));
BLEFloatCharacteristic temperatureCharacteristic("2A6E", BLENotify);
// Call whenever there is an error
void blinkLoop() {
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

String name;
unsigned long lastNotify = 0;

void setup() {
  Serial.begin(9600);

  // check for temperature sensor
  if (!HTS.begin()) {
    Serial.println("Failed to init HTS!");
    blinkLoop();
  }
  if (!BARO.begin()) {
    Serial.println("Failed to init BARO");
    blinkLoop();
  }
  if (!IMU.begin()) {
    Serial.println("Failed init IMU");
    blinkLoop();
  }
  if (!BLE.begin()) {
    Serial.println("Failed to init BLE");
  }
  String address = BLE.address();
  address.toUpperCase();
  Serial.print("address=");
  Serial.println(address);

  name = "BLE Sense - ";
  name += address[address.length() - 5];
  name += address[address.length() - 4];
  name += address[address.length() - 2];
  name += address[address.length() - 1];

  Serial.print("name = ");
  Serial.println(name);

  BLE.setLocalName(name.c_str());
  BLE.setDeviceName(name.c_str());
  BLE.setAdvertisedService(service);

  service.addCharacteristic(versionCharacteristic);
  service.addCharacteristic(accelCharacteristic);
  service.addCharacteristic(gyroCharacteristic);
  service.addCharacteristic(temperatureCharacteristic);
  versionCharacteristic.setValue(VERS);
  BLE.addService(service);
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("incoming connection from..");
    Serial.println(central.address());
  }
  while (central.connected()) {
    updateSubscribedCharacteristics();
    delay(200);
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Connection terminated.");
}

void updateSubscribedCharacteristics() {
  if (accelCharacteristic.subscribed()) {
    float accel[3];
    if (IMU.accelerationAvailable() &&
        IMU.readAcceleration(accel[0], accel[1], accel[2])) {
      accelCharacteristic.writeValue((byte*)accel, sizeof(accel));
    }
    if (gyroCharacteristic.subscribed()) {
      float gyro[3];
      if (IMU.gyroscopeAvailable() &&
          IMU.readGyroscope(gyro[0], gyro[1], gyro[2])) {
        gyroCharacteristic.writeValue((byte*)gyro, sizeof(gyro));
      }
    }
    if (temperatureCharacteristic.subscribed()) {
      float temp = HTS.readTemperature();
      temperatureCharacteristic.writeValue(temp);
    }
  }
}
