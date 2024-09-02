#include <Arduino.h>
#include <BLEDevice.h>

#define SERVER_SERVICE_UUID        "854c4424-7dab-4ddc-8c35-5bebb61ef54f"
#define SERVER_CHARACTERISTIC_UUID "cca12476-18c6-4001-b0f5-6fe1841d862e"
#define BUILT_IN_LED 8
#define SWITCH_PIN 0
#define U_PIN 21
#define D_PIN 20
#define L_PIN 10
#define R_PIN 7
#define M_PIN 6
#define POT_PIN 1

#define BTN_NULL 0x00
#define BTN_RB   0x20
#define BTN_MENU 0x30
#define BTN_LB   0x40
#define BTN_VIEW 0x50

#define BTN_B    0x03
#define BTN_Y    0x04
#define BTN_X    0x05
#define BTN_A    0x06
#define BTN_LEFT 0x0C
#define BTN_UP   0x0D
#define BTN_RIGT 0x0E
#define BTN_DOWN 0x0F

#define PRINT_DEBUG false

// The remote service we wish to connect to.
static BLEUUID serviceUUID(SERVER_SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID(SERVER_CHARACTERISTIC_UUID);
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void printForDebugln(const String& s) {
  if (PRINT_DEBUG) {
    Serial.println(s);
  }
}
static void printForDebug(const String& s) {
  if (PRINT_DEBUG) {
    Serial.print(s);
  }
}

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    printForDebug("Notify callback for characteristic ");
    printForDebug(pBLERemoteCharacteristic->getUUID().toString().c_str());
    printForDebug(" of data length ");
    printForDebugln(length + "");
    printForDebug("data: ");
    printForDebugln((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    if (digitalRead(BUILT_IN_LED) == LOW) {
      digitalWrite(BUILT_IN_LED, HIGH);
    }
    printForDebugln("onDisconnect");
  }
};

// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      printForDebugln("Found our server!");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

bool connectToServer();

bool connectToServer() {
    printForDebug("Forming a connection to ");
    printForDebugln(myDevice->getAddress().toString().c_str());
    BLEClient*  pClient  = BLEDevice::createClient();
    printForDebugln(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());
    // Connect to the remove BLE Server.
    pClient->connect(myDevice);
    printForDebugln(" - Connected to server");
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      printForDebug("Failed to find our service UUID: ");
      printForDebugln(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    printForDebugln(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      printForDebug("Failed to find our characteristic UUID: ");
      printForDebugln(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    printForDebugln(" - Found our characteristic");
    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      printForDebug("The characteristic value was: ");
      printForDebugln(value.c_str());
    }
    connected = true;
    printForDebugln("!!!connected = true");
    return true;
    // if(pRemoteCharacteristic->canNotify())
    //   pRemoteCharacteristic->registerForNotify(notifyCallback);
}


void setup() {
  // put your setup code here, to run once:
  if (PRINT_DEBUG) {
    Serial.begin(115200);
    while(!Serial);
  }
  printForDebugln("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  //  led
  pinMode(BUILT_IN_LED, OUTPUT);
  // switch
  pinMode(SWITCH_PIN, INPUT_PULLDOWN);
  pinMode(U_PIN, INPUT_PULLDOWN);
  pinMode(D_PIN, INPUT_PULLDOWN);
  pinMode(L_PIN, INPUT_PULLDOWN);
  pinMode(R_PIN, INPUT_PULLDOWN);
  pinMode(M_PIN, INPUT_PULLDOWN);
  pinMode(POT_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      printForDebugln("We are now connected to the BLE Server.");
    } else {
      printForDebugln("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
    int potVal = analogRead(POT_PIN);
    printForDebug("POT value=");
    printForDebugln(String(potVal) );
  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    if (digitalRead(BUILT_IN_LED) == HIGH) {
      digitalWrite(BUILT_IN_LED, LOW); // light up led to telling connected
    }
    if (digitalRead(SWITCH_PIN) == HIGH) { // means button is pressed
      pRemoteCharacteristic->writeValue(BTN_A, true);
      printForDebugln("Button A pressed");
    } else if (digitalRead(U_PIN) == HIGH) {
      pRemoteCharacteristic->writeValue(BTN_UP, true);
      printForDebugln("Button UP pressed");
    } else if (digitalRead(D_PIN) == HIGH) {
      pRemoteCharacteristic->writeValue(BTN_DOWN, true);
      printForDebugln("Button DOWN pressed");
    } else if (digitalRead(L_PIN) == HIGH) {
      pRemoteCharacteristic->writeValue(BTN_LEFT, true);
      printForDebugln("Button LEFT pressed");
    } else if (digitalRead(R_PIN) == HIGH) {
      pRemoteCharacteristic->writeValue(BTN_RIGT, true);
      printForDebugln("Button RIGHT pressed");
    } else if (digitalRead(M_PIN) == HIGH) {
      pRemoteCharacteristic->writeValue(BTN_B, true);
      printForDebugln("Button MID pressed");
    } else if (potVal <= 150 || potVal >= 300) {
      // int mappedValue = map(analogRead(POT_PIN), 0, 4095, 0, 255);  // ESP32 has 12-bit ADC (0-4095)
      pRemoteCharacteristic->writeValue(BTN_A, true);
      printForDebugln("Button analog pressed");
    } else {
      pRemoteCharacteristic->writeValue(BTN_NULL, true);
    }
  } else if(doScan){
    BLEDevice::getScan()->start(5);  // this is just an example to re-start the scan after disconnect
  } else {
    delay(1000); // Delay a second between loops.
  }
}
