
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SI_C8051F310_Register_Enums.h>
#include "./src/timer.h"
#include "./src/uart.h"
#include "./src/spi.h"
#include "./src/adc.h"
#include "./src/sys.h"
#include "sta.h"
#include "cmd.h"

extern unsigned char sta_msg_send_complete;
extern unsigned char sta_cmd_send_complete;

extern idata unsigned char cmd_msg[40];

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup (void)
{
   PCA0MD &= ~0x40;                       // WDTE = 0 (clear watchdog timer
}

//-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------
void main (void)
{
	unsigned int adc_var;
	unsigned char adc_target = 0;

	Init_Device();

	while(1)
	{
		if(ADC_Get(&adc_var) == 1)
		{
			if(adc_target == 0)
				Teamperature_Convert(adc_var);
			else Bite_Convert(adc_var);
		}

		if(sta_msg_send_complete == 1)
		{
			State_Msg();
			UART_Send();
		}

		if((Frame2CmdConvert() == 1) && (sta_cmd_send_complete == 1))
			SPI_Send();

		ADC_Start(adc_target);
	}
}
