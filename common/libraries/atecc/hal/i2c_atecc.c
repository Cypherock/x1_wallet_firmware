/**
 * \file
 * \brief  definitions for I2C for Arduino
 *
 * \copyright (c) 2018 Gabriel Notman.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */


#include "atca_hal.h"
#include "atca_iface.h"
#include "atca_status.h"
#include "i2c_atecc.h"

#include <stdio.h>
#include "board.h"
//#include "app_util_platform.h"
//#include "app_error.h"

/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;


ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{    
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
  return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);

    txdata[0] = 0x03;

    txlength++;

    ATCA_STATUS result = ATCA_SUCCESS;

    ret_code_t err_code = BSP_I2C2_IO_Write(cfg->atcai2c.slave_address<<1,txdata,txlength);
    //APP_ERROR_CHECK(err_code);

    if(err_code != STM_SUCCESS)
    {
            return ATCA_COMM_FAIL;
    }
   


  return result;
}

ATCA_STATUS hal_i2c_receive( ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int retries = cfg->rx_retries;
    int status = !ATCA_SUCCESS;
    uint16_t rxdata_max_size = *rxlength;

    ret_code_t err_code;

    *rxlength = 0;

    if (rxdata_max_size < 1) {
        return ATCA_SMALL_BUFFER;
    }


    err_code = BSP_I2C2_IO_Read(cfg->atcai2c.slave_address<<1, rxdata , rxdata_max_size);
    //APP_ERROR_CHECK(err_code);


    if(err_code != STM_SUCCESS)
    {
      status = ATCA_COMM_FAIL;
    }
    else
    {
      status = ATCA_SUCCESS;
    }


    while (retries-- > 0 && (rxdata[0] != (uint8_t)rxdata_max_size ))
    {

            atca_delay_ms(5);
            err_code = BSP_I2C2_IO_Read(cfg->atcai2c.slave_address<<1, rxdata, rxdata_max_size);
            //APP_ERROR_CHECK(err_code);
            retries--;

            if(err_code != STM_SUCCESS)
            {
              status = ATCA_COMM_FAIL;
            }
            else
            {
              status = ATCA_SUCCESS;
            }

    }


    if (status != ATCA_SUCCESS) {
        return (ATCA_STATUS)status;
    }
    if (rxdata[0] < ATCA_RSP_SIZE_MIN) {
        return ATCA_INVALID_SIZE;
    }
    if (rxdata[0] > rxdata_max_size) {
        return ATCA_SMALL_BUFFER;
    }


    if (status != ATCA_SUCCESS) {
        return (ATCA_STATUS)status;
    }

    *rxlength = rxdata[0];

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int retries = cfg->rx_retries;
    uint32_t bdrt = cfg->atcai2c.baud;

    BSP_I2C2_Init(BSP_ATECC_I2C_MODE_STANDARD);

    bool status = true;
    uint8_t data[4];
    memset(data, 0, sizeof(data));

    ret_code_t err_code = BSP_I2C2_IO_Write(cfg->atcai2c.slave_address<<1,data,4);
    //APP_ERROR_CHECK(err_code);


    atca_delay_us(cfg->wake_delay);
//    if(cfg->wake_delay > 1000)
//    {
//    	atca_delay_ms(cfg->wake_delay / 1000);
//    }
//    else
//    {
//    	atca_delay_ms(1);
//    }


    err_code = BSP_I2C2_IO_Read(cfg->atcai2c.slave_address<<1, (uint8_t *)&data, 4);
   // APP_ERROR_CHECK(err_code);

    BSP_I2C2_Init(BSP_ATECC_I2C_MODE_FAST);
    const uint8_t expected[4] = { 0x04, 0x11, 0x33, 0x43 };


    //if (memcmp(data, expected, 4) == 0)
    //{

    //    return ATCA_SUCCESS;

    //}
    
   	return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
   ATCAIfaceCfg *cfg = atgetifacecfg(iface);
 
   ret_code_t err_code = BSP_I2C2_IO_Write( cfg->atcai2c.slave_address<<1,(uint8_t *)0x02,1);
 
   atca_delay_ms(1);
  return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
   ATCAIfaceCfg *cfg = atgetifacecfg(iface);
 
   ret_code_t err_code = BSP_I2C2_IO_Write(cfg->atcai2c.slave_address<<1,(uint8_t *)0x01,1);
 
   atca_delay_ms(1);
  return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_release(void *hal_data )
{
  return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
  return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_devices(int busNum, ATCAIfaceCfg *cfg, int *found )
{
  return ATCA_UNIMPLEMENTED;
}
//#endif






