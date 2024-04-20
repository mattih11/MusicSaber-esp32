#include "LSBT.h"
#include "Arduino.h"

void LSBT::setup() {
  // Create the BLE Device
  BLEDevice::init("LightSaber");

  // Create the BLE Server
  p_server_ = BLEDevice::createServer();
  p_scallbacks_ = new LSBTServerCallbacks();
  p_server_->setCallbacks(p_scallbacks_);

  // Create the BLE Service
  BLEService *p_service = p_server_->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  // we moved to only one service to decrease latency
  // this should stay the preferred way. So additional data should be squeezed
  // into this channel and decoded on receiver side
  p_characteristic_ = p_service->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  p_characteristic_->addDescriptor(new BLE2902());

  // Start the service
  p_service->start();

  // Start advertising
  // TODO: this should be moved to the main loop so we can handle BTLE
  // exceptions without the need to restart the device

  BLEAdvertising *p_advertising = BLEDevice::getAdvertising();
  p_advertising->addServiceUUID(SERVICE_UUID);
  p_advertising->setScanResponse(false);
  p_advertising->setMinPreferred(0x0);  // (taken from example)
                                        // set value to 0x00
                                        // to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

}

// Encode data for transmission
// input: 3 Channels (x,y,z) per sensor (magnetic, acceleration, gyro)
//        = 9 values with 2 bytes, resulting in a payload of 18 bytes
// this function will be inverted on receiver side to get the original data
void LSBT::transform_data(lsm303MagData mag, lsm303AccelData acc,
                          L3G::vector<int16_t> g) {
  data_out_[0] = (uint8_t)(0xFF & mag.x);
  data_out_[1] = (uint8_t)((0xFF00 & mag.x) >> 8);
  data_out_[2] = (uint8_t)(0xFF & mag.y);
  data_out_[3] = (uint8_t)((0xFF00 & mag.y) >> 8);
  data_out_[4] = (uint8_t)(0xFF & mag.z);
  data_out_[5] = (uint8_t)((0xFF00 & mag.z) >> 8);
  data_out_[6] = (uint8_t)(0xFF & acc.x);
  data_out_[7] = (uint8_t)((0xFF00 & acc.x) >> 8);
  data_out_[8] = (uint8_t)(0xFF & acc.y);
  data_out_[9] = (uint8_t)((0xFF00 & acc.y) >> 8);
  data_out_[10] = (uint8_t)(0xFF & acc.z);
  data_out_[11] = (uint8_t)((0xFF00 & acc.z) >> 8);
  data_out_[12] = (uint8_t)(0xFF & g.x);
  data_out_[13] = (uint8_t)((0xFF00 & g.x) >> 8);
  data_out_[14] = (uint8_t)(0xFF & g.y);
  data_out_[15] = (uint8_t)((0xFF00 & g.y) >> 8);
  data_out_[16] = (uint8_t)(0xFF & g.z);
  data_out_[17] = (uint8_t)((0xFF00 & g.z) >> 8);

  Serial.print("data: 0x");
  for (uint8_t *ptr = data_out_; ptr < &data_out_[17]; ptr++) {
    Serial.printf("%x", *ptr);
  }
  Serial.print("\n");
};

// prepare and publish the data via BTLE
void LSBT::publish(lsm303MagData mag, lsm303AccelData acc, L3G::vector<int16_t> g) {
  if (!p_scallbacks_) {
    Serial.printf("no server callbacks initialized -- can't publish");
    return;
  }
  if (p_scallbacks_->device_connected_ ) {
      transform_data(mag, acc, g);
      p_characteristic_->setValue((uint8_t*)&data_out_, 18);
      // indicate changed value, we choose indicate over notify as it's fire
      // and forget and thus latency-friendly ;)
      p_characteristic_->indicate();
      delay(20); // taken from BTLE example:
                 // bluetooth stack will go into congestion,
                 // if too many packets are sent
  }
  // disconnecting
  // TODO: this isn't fully working, when BTLE exceptions occur, we still see
  // high transmission rates on the serial, thus indicating that we don't reach
  // the delay(500) branch of the code
  if (!p_scallbacks_->device_connected_ && !p_scallbacks_->old_device_connected_) {
      delay(500); // give the bluetooth stack the chance to get things ready
      p_server_->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      p_scallbacks_->old_device_connected_ = p_scallbacks_->device_connected_;
  }
  // connecting
  if (p_scallbacks_->device_connected_ && !p_scallbacks_->old_device_connected_) {
      // do stuff here on connecting
      p_scallbacks_->old_device_connected_ = p_scallbacks_->device_connected_;
  }
}

void LSBT::LSBTServerCallbacks::onConnect(BLEServer* p_server) {
  device_connected_ = true;
}

void LSBT::LSBTServerCallbacks::onDisconnect(BLEServer* p_server) {
  device_connected_ = false;
}
