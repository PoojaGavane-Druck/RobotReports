/**
* BHGE Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DSensorDuciPM705E.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     17 April 2020
*
* @brief    The TERPS (TERPS RS485) sensor base class source file
*/
//*********************************************************************************************************************

/* Includes ---------------------------------------------------------------------------------------------------------*/
#include "DSensorDuciTerpsBarometer.h"
#include "DDeviceSerialRS485.h"
#include "DParseMaster.h"

/* Typedefs ---------------------------------------------------------------------------------------------------------*/

/* Defines ----------------------------------------------------------------------------------------------------------*/

/* Macros -----------------------------------------------------------------------------------------------------------*/

/* Variables --------------------------------------------------------------------------------------------------------*/

/* Prototypes -------------------------------------------------------------------------------------------------------*/

/* User code --------------------------------------------------------------------------------------------------------*/
/**
 * @brief   DSensorDuciTerpsBarometer class constructor
 * @param   comms is the reference to the comms instance for this sensor
 * @retval  void
 */
DSensorDuciTerpsBarometer::DSensorDuciTerpsBarometer()
: DSensorDuci()
{
}

/**
 * @brief   Create DUCI Commands required for TERPS pressure sensor
 * @param   void
 * @retval  void
 */
void DSensorDuciTerpsBarometer::createDuciCommands(void)
{
    DSensorDuci::createDuciCommands();

    myParser->addCommand("RE",  "i=4x",         "", fnSetRE,    NULL,   0xFFFFu);   //read sensor error status
//    myParser->addCommand("RF1?",               "!RF1=",  4, {NUMBER, EQUAL_SIGN, STRING, NUMBER, NULL, NULL, NULL, NULL },      DuciSensorSend,            DuciSensorRecv,      10u  },	//RF_CMD_GET_1  		//Read full scale pressure & type
//    myParser->addCommand("RF2?",               "!RF2=",  4, {NUMBER, EQUAL_SIGN, STRING, NUMBER, NULL, NULL, NULL, NULL },      DuciSensorSend,            DuciSensorRecv,      10u  },	//RF_CMD_GET_2  	    //request negative full scale
}
//
///**
// * @brief   Initialisation function
// * @param   void
// * @retval  void
// */
//void DSensorDuciPM705E::initialise(void)
//{
//}

