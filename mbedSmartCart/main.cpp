// ABhi Patel - apatel477@myseneca.ca
// Inderpreet Singh Parmar - iparmar1@myseneca.ca
//main.cpp - This is the main file for running the functions for the hardwares connected to the FRDM k66f
#include "mbed.h"
#include "LCDi2c.h"
#include "MFRC522.h"
#include <map>
#include <string>
 
// --- Hardware Connections ---
LCDi2c lcd(PTB3, PTB2, LCD20x4);                // LCD: SDA, SCL
MFRC522 RfChip(PTD2, PTD3, PTD1, PTC11, PTD0); // RFID: MOSI, MISO, SCK, SDA, RST
UnbufferedSerial esp_uart(PTC4, PTC3, 9600);   // UART RX, TX
DigitalOut greenLED(PTC12);                    // Green LED 
DigitalOut redLED(PTC8);                       // Red LED
PwmOut buzzer(PTC5);                          // PWM Buzzer
DigitalIn captureButton(PTC16);               // Pull up Button (active low)
 
// --- Item Prices --- Our items and price that wil be matched against incoming data from UART
std::map<std::string, float> itemPrices = {
    {"Pop", 1.5}, 
    {"Mug", 5.0}, 
    {"Nutella", 9.99}, 
    {"Apple", 1.5}, 
    {"Orange", 2.0}
};
 
// --- Global variables ---
char buffer[64] = {0};//buffer size is set to 64 char 
int index = 0; // charcter index for buffer
float subtotal = 0.0; //total prices before tax
int itemCount = 0; //item counter
bool systemActive = false; //system inactive by default
bool lastButtonState = true; //button state for capture image everytime
std::string lastItem = ""; //track prev item scanned display purposes
std::string secondLastItem = ""; //track second last item scanned for dispaly purposes
Timer buttonDebounceTimer; //timer to prevent multiple triggers when pressed once
Timer sessionEndTimer; //timeout to factory reset entire system after RFID is tapped to deactivate 
 
// --- PWM Buzzer Function --- Creates audible feedback using PWM, takes 3 variables, freq, time and beep counter to produce customised beep alerts
void beep(float frequency, int duration_ms, int count = 1) {
    for (int i = 0; i < count; i++) {
        buzzer.period(1.0f / frequency);  // Set frequency
        buzzer.write(0.5f);               // 50% duty cycle
        ThisThread::sleep_for(duration_ms);
        buzzer.write(0.0f);               // Turn off
        if (count > 1) ThisThread::sleep_for(100ms); // Gap between beeps
    }
}
 
// --- LCD Update Function --- 
void updateLCD(const char* row1 = "", const char* row2 = "", const char* row3 = "", const char* row4 = "") {
    lcd.cls();
    lcd.locate(0, 0); lcd.printf(row1);
    lcd.locate(0, 1); lcd.printf(row2);
    lcd.locate(0, 2); lcd.printf(row3);
    lcd.locate(0, 3); lcd.printf(row4);
}
 
// --- RFID Detection --- Uses boolean logic to detect system state,  active or inactive
bool detectRFID() {
    if (!RfChip.PICC_IsNewCardPresent() || !RfChip.PICC_ReadCardSerial()) return false;
    RfChip.PICC_HaltA();
    return true;
}
 
// --- Display New Item & Update Total --- THis is where all our maths is happeneing, get item price from item map, update
//                                         update running total, item count and LCD accordingly
void displayItem(const std::string& item) {
    if (itemPrices.count(item)) {
        secondLastItem = lastItem;
        lastItem = item;
 
        itemCount++;
        float price = itemPrices[item];
        subtotal += price;
        float tax = subtotal * 0.13f;
        float total = subtotal + tax;
 
        char row1[21], row2[21], row3[21], row4[21];
        sprintf(row1, sizeof(row1), "%d. %s", itemCount - 1, secondLastItem.c_str());
        sprintf(row2, sizeof(row2), "%d. %s", itemCount, lastItem.c_str());
        sprintf(row3, sizeof(row3), "Tax (13%%): $%.2f", tax);
        sprintf(row4, sizeof(row4), "Total:     $%.2f", total);
 
        updateLCD(row1, row2, row3, row4);
        printf("Item added: %s | Price: $%.2f | Total: $%.2f\n", item.c_str(), price, total);
        beep(2000, 200); // High pitch beep for new item
    } else {
        printf("Unknown item: \"%s\"\n", item.c_str());
        beep(1000, 100, 3); // Triple beep for error
    }
}
 
// --- System Activation/Deactivation --- THis function sets the system state, Inactive or active, and according to that
//                                        it updates our LEDs and LCD. This also activates our camera that is integrated on ESP32
void updateStatusIndicator(bool active) {
    systemActive = active;
    greenLED = active;
    redLED = !active;
 
    if (active) {
        updateLCD("System Active", "Waiting for item...");
        esp_uart.write("S", 1); // Activate ESP32 to start
        printf("Activation sent to ESP32\n");
        beep(1500, 100, 2); // Double beep on activation
    } else {
        char line1[20], line2[20];
        sprintf(line1, sizeof(line1), "Items: %d", itemCount);
        sprintf(line2, sizeof(line2), "Total: $%.2f", subtotal * 1.13f);
        updateLCD("Session Ended", line1, "Thank you!", line2);
        printf("Session Ended. Items: %d | Total: %.2f\n", itemCount, subtotal * 1.13f);
        beep(1200, 300); // Long  beep on deactivation

        // Start timer for welcome screen return
        sessionEndTimer.reset();
        sessionEndTimer.start();
    }
}
 
// --- MAIN ---
int main() {
    memset(buffer, 0, sizeof(buffer)); //Clearing buffer every time the program starts to keep real time operation flow smoothly
    lcd.cls(); //clear lcd
    updateLCD("WELCOME TO SMART CART", "Tap Card to Start");

    RfChip.PCD_Init(); //initialise RFID reader
    updateStatusIndicator(false); //System is inactive by default at start
    buttonDebounceTimer.start();  //timer for capturing picture
 
    while (true) {
        // Check if session end timeout expired and redirect to welcome
        if (!systemActive && sessionEndTimer.elapsed_time() >= 5s) {
            updateLCD("WELCOME TO SMART CART", "Tap Card to Start");
            sessionEndTimer.stop();
        }
 
        // RFID toggle 
        if (detectRFID()) { //if system off, switch on and vice versa
            systemActive = !systemActive;
            updateStatusIndicator(systemActive);
            ThisThread::sleep_for(500ms); 
        }
 
        // Push Button Trigger (debounced)
        bool currentState = captureButton.read(); //read current button state
        if (systemActive && !currentState && lastButtonState && buttonDebounceTimer.elapsed_time() >= 500ms) {
            esp_uart.write("C", 1); //ESP wll capture image is 'C' is sent
            printf("Button Pressed: Requesting capture\n");
            buttonDebounceTimer.reset();
        }
        lastButtonState = currentState; //save button's latest state for next capture
 
        // UART Read Logic (newline-terminated)
        if (esp_uart.readable() && systemActive) {
            char c;
            esp_uart.read(&c, 1);
 
            if (c == '\n' || c == '\r') {
                if (index > 0) { // Only process if data exists
                    buffer[index] = '\0';
                    std::string item(buffer); //convert all buffer items to strings
                    displayItem(item); //Display item and price
                }
                index = 0;
                memset(buffer, 0, sizeof(buffer)); // clear buffer
            } else if (index < sizeof(buffer) - 1) {
                buffer[index++] = c;
            } else {
                printf("UART buffer overflow!\n");
                index = 0;
                memset(buffer, 0, sizeof(buffer)); //buffer will rese if it passes the max of 64 char
            }
        }
 
        ThisThread::sleep_for(50ms); 
    }
}