#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#define MAX_SAVED_SENSORS 	   10u
#define SCALING_CAPTION_SIZE   16u
#define INSTRUMENT_ID_SIZE      16u

//user alarm settings
typedef struct
{
    bool                enabled;                        //user alarm enabled (true = enabled, false = disabled)
    float32_t			high;				            //high threshold setting for user alarm
    float32_t			low;                            //low threshold setting for user alarm

} sAlarmSettings_t;

//filter settings
typedef struct
{
    bool                enabled;                        //filter enabled (true = enabled, false = disabled)
    float32_t			band;				            //threshold within which values are filtered
    float32_t			timeConstant;                   //proportion of change to add at each sample (not really a TC)

} sFilterSettings_t;

//scaling settings
typedef struct
{
    bool                defined;                       //user scaling defined (true = yes, false = no)
    char                caption[SCALING_CAPTION_SIZE]; //threshold within which values are filtered
    sCoordinates_t      coordinates[2];                //two coordinates defining the linear translation to user units

} sScaling_t;

//function measure/source direction
typedef enum
{
    eFunction_t         function;                       //function selected on channel
    eFunctionDir_t      direction;                      //measure/source direction of the function
    eUnits_t            units;                          //units currently selected
    sFilterSettings_t   filter;                         //filter settings
    sAlarmSettings_t    alarm;                          //alarm settings
    sScaling_t          scaling;                        //scaling settings

} eChannelSetting_t;

//channel settings in use with a specific sensor
typedef struct
{
    bool		        free;				            //sensor structure status: true = 'free', false = 'in use'
    uint32_t			serialNumber;                   //sensor serial number
    eChannelSetting_t   channelSetting;                 //channel settings used with sensor having above serial number

} sSensorSettings_t;

/*non-volatile data structure*/
typedef struct
{
    uint32_t			calPin;						    //user calibration PIN
    uint32_t			autoPowerdown;				    //auto-powerdown time in mins (0 = disabled)
    eBacklightMode_t 	backlightMode;				    //backlight operation scheme
    eChannelSetting_t   channel[E_CHANNEL_MAX];         //user functions selection on each channel
    sSensorSettings_t   sensor[MAX_SAVED_SENSORS];      //array of saved channel settings for external sensors
    uint32_t			nextSensorIndex;				//index of next free slot in external sensor array

} sUserSettings_t;

//Persistent data structure
typedef struct
{
    uint32_t			Revision;			 			//Revision of persistent data structure
    sUserSettings_t		UserSettings;			 	 	//user set data

} sInstrumentData_t;

typedef struct
{
    union
    {
        sInstrumentData_t data;                         //Persistent data structure
        char bytes[sizeof(sInstrumentData_t)];          //byte array
    };

    uint32_t crc;                                       //cyclic redundancy check for this data structure

} sPersistentSettings_t;

/*Area of use – region of the world in which the instrument is to be used ------------------------------------------*/
typedef enum
{
    E_REGION_WORLD = 0,		//Rest of the world
    E_REGION_JAPAN,			//Japan

    E_REGION_NUMBER			//NOTE: this must always be the last entry

} eRegionOfUse_t;

/*non-volatile data structure for instrument factory configuration*/
typedef struct
{
    char			serialNumber[INSTRUMENT_ID_SIZE];	//instrument serial number - may be alphanumeric string
    eRegionOfUse_t  region;						        //area of use

} sConfig_t;

typedef struct
{
    union
    {
        sConfig_t	data;                               //configuration data
        char		bytes[sizeof(sConfig_t)];           //byte array
    };

    uint32_t crc;	                                    //cyclic redundancy check for this data structure

} sPersistentConfig_t;

#endif /* __CONFIGURATION_H */

