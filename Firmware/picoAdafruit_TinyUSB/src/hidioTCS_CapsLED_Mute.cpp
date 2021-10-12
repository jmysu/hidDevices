/*********************************************************************
 TCS color sensor w/ HID IO
 by jimmy.su @ 2021

    -HID#1:   TinyUSB generic HIDio 16Bytes
    -HID#2:   HID Keyboard CapsLock LED @pyBaseLED1
    -HID#3:   HID Consumer contrel Mute Key @pyBaseKEY1
    -TCS:     bitbang TCS35725
    -RGBLED:  NeoPixels as TCS's color detector display @pyBaseAD2

*********************************************************************/
#include <Arduino.h>

#include "Adafruit_NeoPixel.h"
#include "Adafruit_TinyUSB.h"
//pyBase AD2:pin28, 8 pixels
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, 28, NEO_GRB + NEO_KHZ800);
//Color Sensor
#include "bitbangTCS34725.h"
extern TCS34725 tcs;

enum {
    REPORT_ID_IO = 1,
    REPORT_ID_KB,
    REPORT_ID_CONSUMER,
};
/*
0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
0x09, 0x01,        // Usage (0x01)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x09, 0x02,        //   Usage (0x02)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0xFF,        //   Logical Maximum (-1)
0x75, 0x08,        //   Report Size (8)
0x95, 0x08,        //   Report Count (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x03,        //   Usage (0x03)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0xFF,        //   Logical Maximum (-1)
0x75, 0x08,        //   Report Size (8)
0x95, 0x08,        //   Report Count (8)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection

0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x02,        //   Report ID (2)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x95, 0x08,        //   Report Count (8)
0x75, 0x01,        //   Report Size (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x08,        //   Report Size (8)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x95, 0x06,        //   Report Count (6)
0x75, 0x08,        //   Report Size (8)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x08,        //   Usage Page (LEDs)
0x19, 0x01,        //   Usage Minimum (Num Lock)
0x29, 0x05,        //   Usage Maximum (Kana)
0x95, 0x05,        //   Report Count (5)
0x75, 0x01,        //   Report Size (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x01,        //   Report Count (1)
0x75, 0x03,        //   Report Size (3)
0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection

0x05, 0x0C,        // Usage Page (Consumer)
0x09, 0x01,        // Usage (Consumer Control)
0xA1, 0x01,        // Collection (Application)
0x85, 0x03,        //   Report ID (3)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
0x19, 0x00,        //   Usage Minimum (Unassigned)
0x2A, 0xFF, 0x03,  //   Usage Maximum (0x03FF)
0x95, 0x01,        //   Report Count (1)
0x75, 0x10,        //   Report Size (16)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
// 126 bytes
*/
// HID report descriptor using TinyUSB's template
// Generic In Out with 8/16 bytes report (max)
#define HIDIO8 8
#define HIDIO16 16
uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_GENERIC_INOUT(HIDIO16, HID_REPORT_ID(REPORT_ID_IO)),  //Report1:HID IO
        TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KB)),                //Report2:KEYBOARD
        TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER)),
};

Adafruit_USBD_HID usb_hid;

static uint8_t get_report_buf[15 + 1] = {0}; /* data to be sent to the PC in response to a GET_REPORT */
static uint8_t getCount = 0;
uint8_t _myReportID;
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    // not used in this example
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    //return 0;

    if (report_type == HID_REPORT_TYPE_FEATURE) {
        char buf[32];
        sprintf(buf, "Get Report(ID:%d)Type:0x%02X:Len:0x%02x>", report_id, report_type, reqlen);  //Not work
        SerialTinyUSB.println(buf);

        get_report_buf[0] = report_id;
        get_report_buf[1] = report_type;
        get_report_buf[2] = reqlen;
        get_report_buf[3] = getCount++;
        get_report_buf[4] = 0xCA;
        get_report_buf[5] = 0xFE;
        get_report_buf[6] = 0xBE;
        get_report_buf[7] = 0xEF;
        buffer = get_report_buf;                                          //assign *buffer
        bool isSent = usb_hid.sendReport(report_id, buffer, reqlen + 1);  //reqlen + report_id
    }
    return reqlen + 1;
}
#define pyBaseKEY1 14
#define pyBaseLED1 18
#define pyBaseLED2 19
#define pyBaseLED3 20

bool isGotReport;
bool isHidReadingTCS = false;
uint8_t lastCmd;
uint8_t red, green, blue, brt;
extern void ledTCS(bool onoff);
uint8_t iTcsTimeINT10 = 40, iTcsGain = 2;
bool isReConfigTCS = false;

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    // This example doesn't use multiple report and report ID
    (void)report_id;
    (void)report_type;
    /*
     * JIMMY: Use the buffer[0] as report_id....
     */
    char buf[32];
    bool isSent;
    int SendSize;
    uint8_t rid;

    if (report_type == HID_REPORT_TYPE_FEATURE) {
        SerialTinyUSB.print("!!!FEATURE!!!");
        SerialTinyUSB.print(" ReportID:");
        SerialTinyUSB.println(report_id);
        rid = report_id;
    } else
        rid = buffer[0];

    _myReportID = rid;
    sprintf(buf, "\nReport(ID:%d)Type:0x%02X:Len:0x%02x>", rid, report_type, bufsize);  //Not work
    SerialTinyUSB.print(buf);
    for (int i = 0; i < bufsize; i++) {
        char buf[8];
        sprintf(buf, "%02X ", *(buffer + i));
        SerialTinyUSB.print(buf);
    }
    SerialTinyUSB.println();
/*
    SendSize = bufsize;
    //isSent = usb_hid.sendReport(rid, buffer, SendSize);

    SerialTinyUSB.print("Echo bytes:");
    SerialTinyUSB.print(SendSize);
    if (isSent)
        SerialTinyUSB.println(" OK");
    else
        SerialTinyUSB.println(" NG");
*/
    if ((bufsize == 2) && (rid == REPORT_ID_KB)) {  // LED indicator is output report with only 1 byte length
        // The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
        // Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
        uint8_t ledIndicator = buffer[1];
        // turn on LED if caplock is set
        digitalWrite(pyBaseLED1, ledIndicator & KEYBOARD_LED_CAPSLOCK);
        SerialTinyUSB.println("!!!KB LED!!!");
    } else {
        if ((bufsize == 8)) {
            lastCmd = buffer[1];
            switch (lastCmd) {
                case 0xF0:  //F0:stop TCS
                    isHidReadingTCS = false;
                    break;
                case 0xF1:  //F1:start TCS
                    isHidReadingTCS = true;
                    break;
                case 0xF6:  //F5:set INT/10
                    iTcsTimeINT10 = buffer[2];
                    isReConfigTCS = true;
                    break;
                case 0xF7:                 //F7:set Gain
                    iTcsGain = buffer[2];  //1~4 => 01~04~16~60
                    isReConfigTCS = true;
                    break;
                case 0xC1:                    //C1:change color from input [2][3][4][5] as RGBA
                    isHidReadingTCS = false;  //Disable TCS color report
                    red = *(buffer + 2);
                    green = *(buffer + 3);
                    blue = *(buffer + 4);
                    brt = *(buffer + 5);
                    break;
                case 0xC2:  //C2:read led color [r][g][b][brt]
                    isHidReadingTCS = false;
                    break;
                case 0xE0:  //E0: off LED
                    ledTCS(0);
                    break;
                case 0xE1:  //E1: on LED
                    ledTCS(1);
                    break;
            }
            isGotReport = true;
        }
    }
}

extern void setupTCS(void);
extern void loopTCS(void);

// the setup function runs once when you press reset or power the board
void setup() {
    //Button Pin14, PULL_HIGH to prevent initialize issue
    pinMode(pyBaseKEY1, INPUT_PULLUP);

#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
    // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
    TinyUSB_Device_Init(0);
    // these two string must be exactly 32 chars long
    //                                       01234567890123456789012345678912
    TinyUSBDevice.setManufacturerDescriptor("JimmyCraft");
    TinyUSBDevice.setProductDescriptor("-TCS HID-");
#endif
    SerialTinyUSB.begin(115200);

    usb_hid.enableOutEndpoint(true);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setReportCallback(get_report_callback, set_report_callback);
    usb_hid.setStringDescriptor("TinyUSB HIDio !");
    usb_hid.begin();

    // wait until device mounted
    while (!TinyUSBDevice.mounted()) delay(1);
    SerialTinyUSB.println("Adafruit TinyUSB HID Generic In Out example");
    pinMode(25, OUTPUT);

    /*
    // This initializes the NeoPixel with RED
    pixels.begin();
    pixels.setBrightness(16);
    pixels.fill(0x880000);
    pixels.show();
    */
    red = 0x88;
    green = 0;
    blue = 0;

    //Setup TCS34725
    setupTCS();

    //Turn LED ON
    pinMode(pyBaseLED1, OUTPUT);
    pinMode(pyBaseLED2, OUTPUT);
    pinMode(pyBaseLED3, OUTPUT);
    digitalWrite(pyBaseLED1, HIGH);
    digitalWrite(pyBaseLED2, HIGH);
    digitalWrite(pyBaseLED3, HIGH);
}

extern TCS34725 tcs;
extern Adafruit_NeoPixel pixels;

int iReportCount = 0;
void loop() {
    static unsigned long lastMillis = 0;
    bool isSent8 = false;
    // Update LED
    uint32_t color;

    if (isGotReport) {
        switch (lastCmd) {
            case 0xC1:
                color = (red << 16) | (green << 8) | blue;
                //pixels.fill(color);
                pixels.clear();
                pixels.show();
                delay(100);
                // Fill the dots one after the other with a color
                pixels.setBrightness(brt);
                for (uint16_t i = 0; i < pixels.numPixels(); i++) {
                    pixels.setPixelColor(i, color);
                    pixels.show();
                    delay(30);
                }
                break;
            case 0xC2:  //Get Neopixel color
                get_report_buf[0] = 0xC2;
                get_report_buf[1] = red;
                get_report_buf[2] = green;
                get_report_buf[3] = blue;
                get_report_buf[4] = brt;
                SerialTinyUSB.print("HID #C2: ");
                SerialTinyUSB.println(_myReportID);

                for (int i = 0; i < 8; i++) {
                    char buf[8];
                    sprintf(buf, "%02X ", *(get_report_buf + i));
                    SerialTinyUSB.print(buf);
                }
                SerialTinyUSB.println();

                isSent8 = usb_hid.sendReport(_myReportID, get_report_buf, HIDIO8);
                break;
        }
        isGotReport = false;
    }

    /*------------- Consumer Control -------------*/
    if (usb_hid.ready()) {
        // Consumer Control is used to control Media playback, Volume, Brightness etc ...
        // Consumer report is 2-byte containing the control code of the key
        // For list of control check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h#L544

        // use to send consumer release report
        static bool has_consumer_key = false;

        if (!digitalRead(pyBaseKEY1)) {
            // toggle MUTE
            usb_hid.sendReport16(REPORT_ID_CONSUMER, HID_USAGE_CONSUMER_MUTE);
            has_consumer_key = true;
        } else {
            // release the consume key by sending zero (0x0000)
            if (has_consumer_key) usb_hid.sendReport16(REPORT_ID_CONSUMER, 0);
            has_consumer_key = false;
        }
    }
    if (isReConfigTCS) {
        //Change integration time w/ iTscTimeINT10
        tcs.integrationTime(iTcsTimeINT10 * 10);
        SerialTinyUSB.print("TCS set INT:");
        SerialTinyUSB.println(iTcsTimeINT10 * 10);
        //Change Gain by iTcsGain
        switch (iTcsGain) {
            case 1:
                tcs.gain(TCS34725::Gain::X01);
                break;
            default:
            case 2:
                tcs.gain(TCS34725::Gain::X04);
                break;
            case 3:
                tcs.gain(TCS34725::Gain::X16);
                break;
            case 4:
                tcs.gain(TCS34725::Gain::X60);
                break;
        }
        isReConfigTCS = false;
    }

    /* Process TCS color report */
    long timeTcsIntegration = tcs.getIntegrationTime();
    if ((millis() - lastMillis) > timeTcsIntegration) {
        lastMillis = millis();
        digitalWrite(25, !digitalRead(25));

        loopTCS();  //update color, neo-pixels, clear interrupt

        //Read TCS
        if (isHidReadingTCS) {
            //iReportCount++;
            //if (iReportCount % 2) {  //Alternate F1/F2 reports
            // Report Color
            TCS34725::Color color = tcs.color();

            get_report_buf[0] = 0xF1;
            get_report_buf[1] = int(color.r);
            get_report_buf[2] = int(color.g);
            get_report_buf[3] = int(color.b);

            int lx = map((int)tcs.lux(), 0, 1024, 0, 255);
            lx = constrain(lx, 0, 255);
            get_report_buf[4] = lx;  //Lux

            int ct = (int)(tcs.colorTemperature() / 100.0f);
            ct = constrain(ct, 0, 255);
            get_report_buf[5] = ct;  //ColorTemp/100

            get_report_buf[6] = (int)(timeTcsIntegration / 10.0f);

            int gain = (int)tcs.getGain();
            get_report_buf[7] = gain;

            //bool isSent8 = usb_hid.sendReport(_myReportID, get_report_buf, 8);
            //} else {
            //----------------------------------------------------------------
            // Report RAW
            TCS34725::RawData craw = tcs.raw();

            //get_report_buf[0] = 0xF2;
            //raw crgb
            get_report_buf[15] = *(craw.raw);  //c
            get_report_buf[14] = *(craw.raw + 1);
            get_report_buf[9] = *(craw.raw + 2);  //r
            get_report_buf[8] = *(craw.raw + 3);
            get_report_buf[11] = *(craw.raw + 4);  //g
            get_report_buf[10] = *(craw.raw + 5);
            get_report_buf[13] = *(craw.raw + 6);  //b
            get_report_buf[12] = *(craw.raw + 7);
            bool isSent16 = usb_hid.sendReport(_myReportID, get_report_buf, HIDIO16);
            //}
        }
    }
}