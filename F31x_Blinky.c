
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
extern unsigned char cmd_msg_valid;

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
	  ADC_Start(adc_target);


//		if(Frame2CmdConvert() == 1)
//		{
//      if(sta_msg_send_complete == 1)
//        UpdateFrame();
//		}

		UART_GetBuff();

		if(cmd_msg_valid == 1)
		{
      SPI_Send();
      cmd_msg_valid = 0;
    }

		if(sta_msg_send_complete == 1)
    {
		  if(Frame2CmdConvert() == 1)
		    UpdateFrame();

      UpdateSta();
      UART_Send();
    }

		if(ADC_Get(&adc_var) == 1)
    {
      if(adc_target == 0)
      {
        Teamperature_Convert(adc_var);
        adc_target = 1;
      }
      else
      {
        Bite_Convert(adc_var);
        adc_target = 0;
      }
    }
	}
}
