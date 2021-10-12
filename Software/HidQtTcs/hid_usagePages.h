#ifndef HID_USAGEPAGES_H
#define HID_USAGEPAGES_H


/* Usage Pages */
enum {
	kHIDPage_Undefined   = 0x00,
	kHIDPage_GenericDesktop = 0x01,
	kHIDPage_Simulation  = 0x02,
	kHIDPage_VR    = 0x03,
	kHIDPage_Sport    = 0x04,
	kHIDPage_Game     = 0x05,
	/* Reserved 0x06 */
	kHIDPage_KeyboardOrKeypad= 0x07, /* USB Device Class Definition for Human Interface Devices (HID). Note: the usage type for all key codes is Selector (Sel). */
	kHIDPage_LEDs     = 0x08,
	kHIDPage_Button      = 0x09,
	kHIDPage_Ordinal  = 0x0A,
	kHIDPage_Telephony   = 0x0B,
	kHIDPage_Consumer = 0x0C,
	kHIDPage_Digitizer   = 0x0D,
	/* Reserved 0x0E */
	kHIDPage_PID      = 0x0F,  /* USB Physical Interface Device definitions for force feedback and related devices. */
	kHIDPage_Unicode  = 0x10,
	/* Reserved 0x11 - 0x13 */
	kHIDPage_AlphanumericDisplay= 0x14,
	/* Reserved 0x15 - 0x7F */
	/* Monitor 0x80 - 0x83	 USB Device Class Definition for Monitor Devices */
	/* Power 0x84 - 0x87	 USB Device Class Definition for Power Devices */
	kHIDPage_PowerDevice = 0x84,           /* Power Device Page */
	kHIDPage_BatterySystem  = 0x85,           /* Battery System Page */
	/* Reserved 0x88 - 0x8B */
	kHIDPage_BarCodeScanner = 0x8C,  /* (Point of Sale) USB Device Class Definition for Bar Code Scanner Devices */
	kHIDPage_Scale = 0x8D,  /* (Point of Sale) USB Device Class Definition for Scale Devices */
	/* ReservedPointofSalepages 0x8E - 0x8F */
	kHIDPage_CameraControl  = 0x90,  /* USB Device Class Definition for Image Class Devices */
	kHIDPage_Arcade   = 0x91,  /* OAAF Definitions for arcade and coinop related Devices */
	/* Reserved 0x92 - 0xFEFF */
	/* VendorDefined 0xFF00 - 0xFFFF */
	kHIDPage_VendorDefinedStart   = 0xFF00
};


#endif // HID_USAGEPAGES_H
