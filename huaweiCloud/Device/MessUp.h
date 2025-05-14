
// 定义发送消息的地址域
typedef enum {
	Mess_Adder_settledFlow        = 0x00,
	Mess_Adder_currentFlow        = 0x01,
	Mess_Adder_No                 = 0x02,
	Mess_Adder_IMEI               = 0x03,
	Mess_Adder_NCCID              = 0x04,
	Mess_Adder_CSQ                = 0x05,
	Mess_Adder_IMSI               = 0x06,
	Mess_Adder_report_failed      = 0x07,
	Mess_Adder_report_success     = 0x08,
	Mess_Adder_CellID             = 0x09,
	Mess_Adder_SNR                = 0x0A,
	Mess_Adder_ECR                = 0x0B,
	Mess_Adder_RSRP               = 0x0C,
	Mess_Adder_voltage            = 0x0D,
	Mess_Adder_TXPower            = 0x0E,
	Mess_Adder_Date               = 0x0F,
	Mess_Adder_SEQ                = 0x10,
	Mess_Adder_devicestatus       = 0x11,
	Mess_Adder_flash              = 0x12,
	Mess_Adder_Metering           = 0x13,
	Mess_Adder_battery_door       = 0x14,
	Mess_Adder_Encrypt            = 0x15,
	Mess_Adder_Infrared           = 0x16,
	Mess_Adder_button             = 0x17,
	Mess_Adder_Escalation_cycle   = 0x18
} Mess_Adder_t;

// 定义发送消息的长度，一个长度是两位
typedef enum {
	Mess_Length_settledFlow        = 10,
	Mess_Length_currentFlow        = 10,
	Mess_Length_No                 = 14,
	Mess_Length_IMEI               = 15,
	Mess_Length_NCCID              = 20,
	Mess_Length_CSQ                = 5,
	Mess_Length_IMSI               = 15,
	Mess_Length_report_failed      = 5,
	Mess_Length_report_success     = 5,
	Mess_Length_CellID             = 8,
	Mess_Length_SNR                = 5,
	Mess_Length_ECR                = 5,
	Mess_Length_RSRP               = 5,
	Mess_Length_TXPower            = 5,
	Mess_Length_voltage            = 5,
	Mess_Length_Date               = 19,
	Mess_Length_SEQ                = 20,
	Mess_Length_devicestatus       = 1,
	Mess_Length_flash              = 1,
	Mess_Length_Metering           = 1,
	Mess_Length_battery_door       = 1,
	Mess_Length_Encrypt            = 1,
	Mess_Length_Infrared           = 1,
	Mess_Length_button             = 1,
	Mess_Length_Escalation_cycle   = 5
} Mess_Length_t;

void StringToHexASCII(const char *input, int inputLength, char *output, int outputLength);
void SendMess_Up_Char(Mess_Adder_t MessAdder, char *MessContent, int MessLength, Mess_Length_t MessFixedLength) ;