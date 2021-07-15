#ifndef __RPMSG_CONNECTOR_COMMON
#define __RPMSG_CONNECTOR_COMMON

#include "stdint.h"

// #pragma enum(1)
#pragma pack(push, 4)

#if !defined(__DRIVER_HEADER)

///общая структура системных данным структуры модуля
typedef struct{
  uint8_t   InWork;
  uint8_t   ModuleIndex;                                                        ///<позиция структуры для указателей на массивы modbus           
}ModuleInfo_str;

///общая структура системных данным структуры модуля
typedef struct{  
  uint16_t  NodeID;                                                             ///<id canopen сети
  uint32_t  IDVendor;                                                           ///<id устройства, 4 шт
  uint32_t  IDProdCode;
  uint32_t  IDRevision;
  uint32_t  IDSerial;
  uint8_t   HeatrbeatState;                                                     ///<NMT состояние
  uint8_t   HeatrbeatFl;                                                        ///<флаг пришедшего хартбита
  uint32_t  PDOCounter;                                                    
  uint8_t   PDOCycle;    
}ModuleSysData_str;

#define UL_IN_PROCESS  (0xFFFF)

typedef enum UL_RESULT_Enum{
  UL_OK = 0,
  UL_SUCCESS,
  UL_ERROR,
  UL_NO_OPERATION,
  UL_CAN_SDO_ERROR,            
  UL_INDEX_NOT_AVAIL, UL_MODULE_DISCONNECTED, UL_TYPE_NOT_AVAIL, 
  UL_MEM_OVERFLOW,    UL_MEM_NOT_AVAIL, 
  UL_MB_WRONG_COM,    UL_MB_WRONG_TYPE,  UL_MB_LOW_TIMEOUT,    UL_MB_WRONG_INDEX,
  UL_FSM_TIMEOUT,     UL_FSM_NOT_AVAIL,  UL_FSM_NOT_EDIT_MODE, UL_FSM_NO_FILE,     UL_FSM_NO_PATH,
  UL_I2C_TIMEOUT,     UL_I2C_NOT_AVAIL
} UL_RESULT __attribute__ ((mode(__byte__)));

///идентификаторы типов модулей
typedef enum{
  DevTypeK15_AI8      = 0x0100,
  DevTypeK15_AO2      = 0x0200,
  DevTypeK15_DI16     = 0x0300,
  DevTypeK15_DO16     = 0x0400,
  DevTypeK15_NAMUR    = 0x0500,
  DevTypeK15_AI6      = 0x0600, 
  DevTypeK19_AI6p     = 0x0700,
  DevTypeK19_AI6a     = 0x0800,
  DevTypeK15_DO8pwr   = 0x0900,
  DevTypeK19_DI16     = 0x0A00,
  DevTypeK19_DO16     = 0x0B00,
  DevTypeK19_FIO      = 0x0C00,
  DevTypeK19_CRM      = 0x0D00,
  DevTypeK19_AO4      = 0x0E00,
  DevTypeK19_SCM      = 0x0F00,
  DevTypeK19_DIDO8    = 0x1000,
  DevTypeK15_DO8      = 0x1100,
  DevTypeK15_FDI8     = 0x1200,
  DevTypeUNKNOWN      = 0xFF00
} DeviceType  __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum { 
  UL_K15_DO16_OutLow  = 0,
  UL_K15_DO16_OutHigh = 1 
} UL_K15_DO16_OutState __attribute__ ((mode(__byte__)));

typedef enum { 
  UL_K15_DO16_NoSc    = 0, 
  UL_K15_DO16_ScOccur = 1 
}UL_K15_DO16_ScState __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K15_DO16
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K15_DO16_OutState    Out[16];                                            ///<считанные значения текущих выходных состояний каналов
    UL_K15_DO16_ScState     Sc[16];                                             ///<считанные значения короткого замыкания каналов
    float                   Frequency[12];                                      ///<считанные значения частот каналов. частоты считаются только для каналов 1-4,9-12. промежуточные каналы не активны
    uint32_t                Counter[12];                                        ///<считанные значения счетчиков каналов. —четчики считаются только для каналов 1-4,9-12. промежуточные каналы не активны
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{                                                                       
    UL_K15_DO16_OutState    Out[16];                                            ///<значения выходных состояний каналов
  }Write;    
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_DO16_USE_SIMULATION > 0
    struct{
      uint8_t   Out[16];
      uint8_t   Sc[16];      
    }Simulation;
  #endif  
} K15_DO16_str;


/*enums************************************************************************/
///каналы
typedef enum { 
  UL_K15_DI16_CH1=0, UL_K15_DI16_CH2,  UL_K15_DI16_CH3,  UL_K15_DI16_CH4,
  UL_K15_DI16_CH5,   UL_K15_DI16_CH6,  UL_K15_DI16_CH7,  UL_K15_DI16_CH8,                 
  UL_K15_DI16_CH9,   UL_K15_DI16_CH10, UL_K15_DI16_CH11, UL_K15_DI16_CH12,               
  UL_K15_DI16_CH13,  UL_K15_DI16_CH14, UL_K15_DI16_CH15, UL_K15_DI16_CH16 
}UL_K15_DI16_Channels __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum {
  UL_K15_DI16_ChStLow  = 0, 
  UL_K15_DI16_ChStHigh = 1 
}UL_K15_DI16_ChState __attribute__ ((mode(__byte__)));    

///входной фильтр канала
typedef enum {
  UL_K15_DI16_Filter100Hz  = 3,
  UL_K15_DI16_Filter250Hz  = 2,
  UL_K15_DI16_Filter1000Hz = 1,
  UL_K15_DI16_FilterNone   = 0 
}UL_K15_DI16_ChFilter __attribute__ ((mode(__byte__)));

typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K15_DI16_ChState     In[16];                                             ///<считанные состояния входов каналов
    UL_K15_DI16_ChFilter    Filter[16];                                         ///<считанные значения фильтров каналов
    uint32_t                Counter[16];                                        ///<считанные значения счетчиков каналов
    float                   Frequency[12];                                      ///<считанные значения частот каналов. частоты считаются только для каналов 1-4,9-12. промежуточные каналы не активны
    struct{                                                                     ///<статусы
      uint16_t  CAN, Special, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{
    UL_K15_DI16_ChFilter    Filter[16];                                         ///<значения фильтров каналов
  } Write;
  ///системная информация 
  ModuleSysData_str System;                                                     ///<системная информация
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_DI16_USE_SIMULATION > 0
    struct{
      uint8_t   In[16];
      uint8_t   Counter[16];
      uint8_t   Frequency[12];                                                  ///<симуляция частоты активна только для каналов 1-4,9-12. промежуточные каналы не активны
    }Simulation;
  #endif   
}K15_DI16_str;


/*enums************************************************************************/
///состояние выдачи АЦП каналов
typedef enum{
  UL_K15_AI8_ADC_Disabled = 0,
  UL_K15_AI8_ADC_Enabled  = 1
}UL_K15_AI8_ADC_Status __attribute__ ((mode(__byte__)));

/*structs**********************************************************************/
///структура модуля K15_AI8
typedef struct{
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    uint16_t               Adc[8];                                              ///<АЦП каналов. Читается только с включеным ADCEnable
    float                  Data[8];                                             ///<данные каналов в мА
    float                  Temp;                                                ///<температура модуля, °C
    UL_K15_AI8_ADC_Status  ADCEnable;                                           ///<флаг отправки показаний АЦП
    ///статусы  
    struct{
      uint16_t  CAN, Firmware, Hardware;    
    }Status;  
  }Read;
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. Выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_AI8_USE_SIMULATION > 0
    struct{
      uint8_t Adc[8];
      uint8_t Data[8];    
    }Simulation;
  #endif
} K15_AI8_str;

///использовать функции калибровки каналов
#define UL_K19_AO4_CALIB_SEQ        1

/*enums************************************************************************/
///состояние канала
typedef enum {
  UL_K15_AO2_Disable = 0,
  UL_K15_AO2_Enable  = 1 
}UL_K15_AO2_ChEnable __attribute__ ((mode(__byte__)));

///тип выходного сигнала K15_AO2
typedef enum {
  UL_K15_AO2_Voltage = 0, 
  UL_K15_AO2_Current = 1 
}UL_K15_AO2_ChSource __attribute__ ((mode(__byte__)));

///<тестовый режим K15_AO2
typedef enum { 
  UL_K15_AO2_TestmodeOff = 0,
  UL_K15_AO2_TestmodeOn  = 1 
}UL_K15_AO2_ChTestmode __attribute__ ((mode(__byte__)));  

/*structures*******************************************************************/
///структура модуля K15_AO2
typedef struct{
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    float                   Out[2];                                             ///<выходное значение мА или В
    UL_K15_AO2_ChEnable     Enable[2];                                          ///<флаг включения канала
    UL_K15_AO2_ChSource     Source[2];                                          ///<тип выходного источника ток или напряжение
    UL_K15_AO2_ChTestmode   Test[2];                                            ///<флаг включения тестового режима канала
    float                   Calb[2];                                            ///<значения калибровки   
    struct{                                                                     ///<структура статус параметров
      uint16_t CAN, Firmware, Hardware, Dac1, Dac2, SPI;    
    }Status; 
  }Read;
  ///данные автоматической записи. Достаточно изменить значение в структуре Write
  struct{
    float                   Out[2];                                             ///<выходное значение мА или В
    UL_K15_AO2_ChEnable     Enable[2];                                          ///<флаг включения канала
    UL_K15_AO2_ChSource     Source[2];                                          ///<тип выходного источника ток или напряжение
    UL_K15_AO2_ChTestmode   Test[2];                                            ///<флаг включения тестового режима канала
  }Write;
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. Выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_AO2_USE_SIMULATION > 0
    struct{
      uint8_t   Out[2];
    }Simulation;
  #endif  
} K15_AO2_str;

/*enums************************************************************************/
///состояние выдачи АЦП каналов
typedef enum{
  UL_K15_AI6_ADC_Disabled = 0,
  UL_K15_AI6_ADC_Enabled  = 1
}UL_K15_AI6_ADC_Status __attribute__ ((mode(__byte__)));

///тип источника измерения 0 - ток, 1 - напряжение. Выставляется механически на DIP переключателе модуля
typedef enum{
  UL_K15_AI6_SourceCurrent = 0,
  UL_K15_AI6_SourceVoltage = 1
}UL_K15_AI6_Source __attribute__ ((mode(__byte__)));

///<тип измерения 0 - биполярный (-24..24 мА, -10..10 В), 1 - униполярный (0..24 мА, 0..10 В)
typedef enum{
  UL_K15_AI6_MeasTypeBipolar  = 0,
  UL_K15_AI6_MeasTypeUnipolar = 1
}UL_K15_AI6_MeasType __attribute__ ((mode(__byte__)));

/*structs**********************************************************************/
///структура модуля K15_AI6
typedef struct{
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    uint16_t                Adc[6];                                             ///<АЦП каналов
    float                   Data[6];                                            ///<данные в мА или В в зависимости от Source
    UL_K15_AI6_Source       Source[6];                                          ///<тип источника измерения
    UL_K15_AI6_MeasType     Type[6];                                            ///<тип измерения
    float                   Temp;                                               ///<температура модуля, °C 
    UL_K15_AI6_ADC_Status   ADCEnable;                                          ///<флаг отправки показаний АЦП
    ///статусы  
    struct{
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. Достаточно изменить значение в структуре Write
  struct{
    UL_K15_AI6_MeasType     Type[6];                                            ///<тип измерения    
  }Write;  
  ///системная информация 
  ModuleSysData_str System;                                                     ///<системная информация 
  ///флаги симуляции значений. Выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_AI6_USE_SIMULATION > 0
    struct{
      uint8_t Adc[8];
      uint8_t Data[8];    
    }Simulation;
  #endif
} K15_AI6_str;


/*enums************************************************************************/
///каналы K15_DO8pwr
typedef enum { 
  UL_K15_DO8pwr_Ch1, UL_K15_DO8pwr_Ch2, UL_K15_DO8pwr_Ch3, UL_K15_DO8pwr_Ch4,
  UL_K15_DO8pwr_Ch5, UL_K15_DO8pwr_Ch6, UL_K15_DO8pwr_Ch7, UL_K15_DO8pwr_Ch8
}UL_K15_DO8pwr_Channels __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum { 
  UL_K15_DO8pwr_OutLow  = 0,
  UL_K15_DO8pwr_OutHigh = 1 
}UL_K15_DO8pwr_OutState __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K15_DO8pwr
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K15_DO8pwr_OutState Out[8];                                              ///<считанные значения текущих выходных состояний каналов
    float                  Frequency[8];                                        ///<считанные значения частот каналов.
    uint32_t               Counter[8];                                          ///<считанные значения счетчиков каналов.
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{                                                                       
    UL_K15_DO8pwr_OutState    Out[8];                                           ///<значения выходных состояний каналов
  }Write;    
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_DO8pwr_USE_SIMULATION > 0
    struct{
      uint8_t   Out[8];     
    }Simulation;
  #endif  
}K15_DO8pwr_str;

// #define K15_DO8_str     K15_DO8pwr_str

/*enums************************************************************************/
///состояние входов K15_NAMUR
typedef enum { 
  UL_K15_NAM_ChLow  = 0, 
  UL_K15_NAM_ChHigh = 1 
}UL_K15_NAM_ChState __attribute__ ((mode(__byte__)));

///состояние короткого замыкания входов K15_NAMUR
typedef enum { 
  UL_K15_NAM_NoSc = 0, 
  UL_K15_NAM_ScOccur = 1 
}UL_K15_NAM_ScState __attribute__ ((mode(__byte__)));

///состояние подключения датчика входов K15_NAMUR
typedef enum { 
  UL_K15_NAM_NoWb = 0, 
  UL_K15_NAM_WbOccur = 1 
}UL_K15_NAM_WbState __attribute__ ((mode(__byte__)));

///состояние инвертирования входов K15_NAMUR
typedef enum { 
  UL_K15_NAM_InversionOff = 0, 
  UL_K15_NAM_InversionOn = 1 
}UL_K15_NAM_Inversion __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K15_NAM
typedef struct{
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K15_NAM_ChState      In[4];                                              ///<текущее состояние канала
    UL_K15_NAM_ScState      Sc[4];                                              ///<короткое замыкание на канале
    UL_K15_NAM_WbState      Wb[4];                                              ///<обыв фазы на канале
    UL_K15_NAM_Inversion    Inv[4];                                             ///<инверсия входного сигнала
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware, I2CErr, StatI2C;    
    }Status; 
  }Read;
  ///данные автоматической записи. Достаточно изменить значение в структуре Write
  struct{
    UL_K15_NAM_Inversion    Inv[4];
  }Write;
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. Выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_NAM_USE_SIMULATION > 0
    struct{
      uint8_t   In[4];
      uint8_t   Sc[4];
      uint8_t   Wb[4];
    }Simulation;
  #endif  
}K15_NAM_str;


/*enums************************************************************************/
///типы обработки сигнала
typedef enum{
  K19_AI6p_ChType_mA_0_20 = 0,                                                  ///<Ток. диапазон 0 - 20 мА
  K19_AI6p_ChType_mA_4_20,                                                      ///<Ток. диапазон 4 - 20 мА
  K19_AI6p_ChType_V_m10_10,                                                     ///<Напряжение. диапазон -10 - 10 В
  K19_AI6p_ChType_V_0_10                                                        ///<Напряжение. диапазон 0 - 10 В
}K19_AI6p_ChType __attribute__ ((mode(__byte__)));

///тип выходного значения АЦП через PDO
typedef enum{
  K19_AI6p_OutType_ReducedADC = 0,                                              ///<в PDO выдаются приведенные значения АЦП где 0мА(-11В) - 0АЦП, 24мА(11В) - 65535АЦП
  K19_AI6p_OutType_TrueADC                                                      ///<в PDO выдаются считанные значения АЦП
}K19_AI6p_OutADCType __attribute__ ((mode(__byte__)));

///тип выходного значения данных через PDO
typedef enum{
  K19_AI6p_OutType_ReducedData = 0,                                             ///<в PDO выдаются приведенные значения 
  K19_AI6p_OutType_TrueData,                                                    ///<в PDO выдаются занные до пользовательской коррекции
}K19_AI6p_OutDataType __attribute__ ((mode(__byte__)));

///поведение красного светодиода канала
typedef enum{
  K19_AI6p_RedLEDNormal = 0,                                                    ///<загорается когда значение вне диапазона
  K19_AI6p_RedLEDBlocked                                                        ///<заблокирован. Не загорается
}K19_AI6p_RedLEDAction __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum{
  K19_AI6p_StateCh_Normal = 0,                                                  ///<канал работает
  K19_AI6p_StateCh_Alarm                                                        ///<авария канала
}K19_AI6p_StateCh __attribute__ ((mode(__byte__)));

///состояние типа входа
typedef enum{
  K19_AI6p_StateType_mA = 0,                                                    ///<токовый канал
  K19_AI6p_StateType_V                                                          ///<канал напряжения
}K19_AI6p_StateType __attribute__ ((mode(__byte__)));

///состояние типа входа
typedef enum{
  K19_AI6p_StateActive_False = 0,                                               ///<пассивный канал
  K19_AI6p_StateActive_True                                                     ///<активный канал
}K19_AI6p_StateActive __attribute__ ((mode(__byte__)));

///состояние красного светодиода канала
typedef enum{
  K19_AI6p_StateRedLED_OFF = 0,                                                 ///<красный светодиод не горит
  K19_AI6p_StateRedLED_ON                                                       ///<красный светодиод горит
}K19_AI6p_StateRedLED __attribute__ ((mode(__byte__)));

/*structs**********************************************************************/
///структура модуля K19_AI6p
typedef struct{
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    uint16_t                 Adc[6];                                            ///<АЦП каналов
    float                    Data[6];                                           ///<данные в мА или В в зависимости от Source
    K19_AI6p_ChType          ChType[6];                                         ///типы обработки сигнала
    K19_AI6p_OutADCType      OutADCType[6];                                     ///тип получаемого значения АЦП 
    K19_AI6p_OutDataType     OutDataType[6];                                    ///тип получаемого значения Data   
    K19_AI6p_RedLEDAction    RedLEDAction[6];                                   ///поведение красного светодиода канала    
    K19_AI6p_StateCh         StateCh[6];                                        ///состояние канала
    K19_AI6p_StateType       StateType[6];                                      ///состояние типа входа
    K19_AI6p_StateActive     StateActive[6];                                    ///состояние типа входа
    K19_AI6p_StateRedLED     StateRedLED[6];                                    ///состояние красного светодиода канала
    float                    Temp;                                              ///<температура модуля, °C 
    ///статусы  
    struct{
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. Достаточно изменить значение в структуре Write
  struct{
    K19_AI6p_ChType          ChType[6];                                         ///типы обработки сигнала
    K19_AI6p_OutADCType      OutADCType[6];                                     ///тип получаемого значения АЦП 
    K19_AI6p_OutDataType     OutDataType[6];                                    ///тип получаемого значения Data   
    K19_AI6p_RedLEDAction    RedLEDAction[6];                                   ///поведение красного светодиода канала   
  }Write;  
  ///системная информация 
  ModuleSysData_str System;                                                     ///<системная информация 
  ///флаги симуляции значений. Выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_AI6p_USE_SIMULATION > 0
    struct{
      uint8_t Adc[6];
      uint8_t Data[6];    
    }Simulation;
  #endif
} K19_AI6p_str;

/*enums************************************************************************/
///типы обработки сигнала
typedef enum{
  K19_AI6a_ChType_mA_0_20 = 0,                                                  ///<Ток. диапазон 0 - 20 мА
  K19_AI6a_ChType_mA_4_20,                                                      ///<Ток. диапазон 4 - 20 мА
}K19_AI6a_ChType __attribute__ ((mode(__byte__)));

///тип выходного значения АЦП через PDO
typedef enum{
  K19_AI6a_OutType_ReducedADC = 0,                                              ///<в PDO выдаются приведенные значения АЦП где 0мА(-11В) - 0АЦП, 24мА(11В) - 65535АЦП
  K19_AI6a_OutType_TrueADC                                                      ///<в PDO выдаются считанные значения АЦП
}K19_AI6a_OutADCType __attribute__ ((mode(__byte__)));

///тип выходного значения данных через PDO
typedef enum{
  K19_AI6a_OutType_ReducedData = 0,                                             ///<в PDO выдаются приведенные значения 
  K19_AI6a_OutType_TrueData,                                                    ///<в PDO выдаются занные до пользовательской коррекции
}K19_AI6a_OutDataType __attribute__ ((mode(__byte__)));

///поведение красного светодиода канала
typedef enum{
  K19_AI6a_RedLEDNormal = 0,                                                    ///<загорается когда значение вне диапазона
  K19_AI6a_RedLEDBlocked                                                        ///<заблокирован. Не загорается
}K19_AI6a_RedLEDAction __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum{
  K19_AI6a_StateCh_Normal = 0,                                                  ///<канал работает
  K19_AI6a_StateCh_Alarm                                                        ///<авария канала
}K19_AI6a_StateCh __attribute__ ((mode(__byte__)));

///состояние типа входа
typedef enum{
  K19_AI6a_StateActive_False = 0,                                               ///<пассивный канал
  K19_AI6a_StateActive_True                                                     ///<активный канал
}K19_AI6a_StateActive __attribute__ ((mode(__byte__)));

///состояние красного светодиода канала
typedef enum{
  K19_AI6a_StateRedLED_OFF = 0,                                                 ///<красный светодиод не горит
  K19_AI6a_StateRedLED_ON                                                       ///<красный светодиод горит
}K19_AI6a_StateRedLED __attribute__ ((mode(__byte__)));

/*structs**********************************************************************/
///структура модуля K19_AI6a
typedef struct{
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    uint16_t                 Adc[6];                                            ///<АЦП каналов
    float                    Data[6];                                           ///<данные в мА или В в зависимости от Source
    K19_AI6a_ChType          ChType[6];                                         ///типы обработки сигнала
    K19_AI6a_OutADCType      OutADCType[6];                                     ///тип получаемого значения АЦП 
    K19_AI6a_OutDataType     OutDataType[6];                                    ///тип получаемого значения Data   
    K19_AI6a_RedLEDAction    RedLEDAction[6];                                   ///поведение красного светодиода канала    
    K19_AI6a_StateCh         StateCh[6];                                        ///состояние канала
    K19_AI6a_StateActive     StateActive[6];                                    ///состояние типа входа
    K19_AI6a_StateRedLED     StateRedLED[6];                                    ///состояние красного светодиода канала
    float                    Temp;                                              ///<температура модуля, °C 
    ///статусы  
    struct{
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. Достаточно изменить значение в структуре Write
  struct{
    K19_AI6a_ChType          ChType[6];                                         ///типы обработки сигнала
    K19_AI6a_OutADCType      OutADCType[6];                                     ///тип получаемого значения АЦП 
    K19_AI6a_OutDataType     OutDataType[6];                                    ///тип получаемого значения Data   
    K19_AI6a_RedLEDAction    RedLEDAction[6];                                   ///поведение красного светодиода канала   
  }Write;  
  ///системная информация 
  ModuleSysData_str System;                                                     ///<системная информация 
  ///флаги симуляции значений. Выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_AI6a_USE_SIMULATION > 0
    struct{
      uint8_t Adc[6];
	  uint8_t Data[6];
    }Simulation;
  #endif
} K19_AI6a_str;

/*enums************************************************************************/
///каналы
typedef enum { 
  UL_K19_DI16_CH1=0, UL_K19_DI16_CH2,  UL_K19_DI16_CH3,  UL_K19_DI16_CH4,
  UL_K19_DI16_CH5,   UL_K19_DI16_CH6,  UL_K19_DI16_CH7,  UL_K19_DI16_CH8,                 
  UL_K19_DI16_CH9,   UL_K19_DI16_CH10, UL_K19_DI16_CH11, UL_K19_DI16_CH12,               
  UL_K19_DI16_CH13,  UL_K19_DI16_CH14, UL_K19_DI16_CH15, UL_K19_DI16_CH16 
}UL_K19_DI16_Channels __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum {
  UL_K19_DI16_ChStLow  = 0, 
  UL_K19_DI16_ChStHigh = 1 
}UL_K19_DI16_ChState __attribute__ ((mode(__byte__)));    

/*structures*******************************************************************/
///структура модуля K19_DI16
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K19_DI16_ChState     In[16];                                             ///<считанные состояния входов каналов    
    uint32_t                Counter[16];                                        ///<считанные значения счетчиков каналов
    uint16_t                Filter;                                             ///<считанные значения фильтра в мс для модуля. работает для всех каналов. 10 - 1000 мс. 100 по умолчанию
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{
    uint16_t                Filter;                                             ///<значения фильтра модуля. 10 - 1000 мс. 100 по умолчанию 
  }Write;
  ///системная информация 
  ModuleSysData_str System;                                                     ///<системная информация
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_DI16_USE_SIMULATION > 0
    struct{
      uint8_t   In[16];
      uint8_t   Counter[16];
    }Simulation;
  #endif   
}K19_DI16_str;

/*enums************************************************************************/
///каналы K19_DO16
typedef enum { 
  UL_K19_DO16_Ch1, UL_K19_DO16_Ch2, UL_K19_DO16_Ch3, UL_K19_DO16_Ch4,
  UL_K19_DO16_Ch5, UL_K19_DO16_Ch6, UL_K19_DO16_Ch7, UL_K19_DO16_Ch8,  
  UL_K19_DO16_Ch9, UL_K19_DO16_Ch10,UL_K19_DO16_Ch11,UL_K19_DO16_Ch12,
  UL_K19_DO16_Ch13,UL_K19_DO16_Ch14,UL_K19_DO16_Ch15,UL_K19_DO16_Ch16 
}UL_K19_DO16_Channels __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum { 
  UL_K19_DO16_OutLow  = 0,
  UL_K19_DO16_OutHigh = 1 
}UL_K19_DO16_OutState __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K19_DO16
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K19_DO16_OutState    Out[16];                                            ///<считанные значения текущих выходных состояний каналов
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{                                                                       
    UL_K19_DO16_OutState    Out[16];                                            ///<значения выходных состояний каналов
  }Write;    
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_DO16_USE_SIMULATION > 0
    struct{
      uint8_t   Out[16];
      uint8_t   Sc[16];      
    }Simulation;
  #endif  
}K19_DO16_str;

/*enums************************************************************************/
///тип задаваемого значения
typedef enum{
  K19_AO4_DST_ReducedDAC = 0,                                                   ///<значение управления - приведенный отсчет ЦАП 0мА - 0, 24мА - 65535
  K19_AO4_DST_TrueUA                                                            ///<значение управления - ток в мкА
}K19_AO4_DataSetType __attribute__ ((mode(__byte__)));

///тип выходного канала
typedef enum{
  K19_AO4_CT_Active = 0,                                                        ///<активный канал с напряжением на выходе
  K19_AO4_CT_Passive                                                            ///<пассивный канал без напряжения для токовой петли
}K19_AO4_ChType __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K19_AO4
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    K19_AO4_DataSetType   DataSetType[4];                                       ///<тип данных канала: ток в мкА или приведенный ЦАП
    K19_AO4_ChType        ChType[4];                                            ///<тип канала: активный/пассивный.
    uint16_t              Data[4];                                              ///<выходное значение, соотвествующее DataSetType с учетом диапазонов в приведенном отсчете ЦАП или токе в мкА
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{                                                                       
    K19_AO4_DataSetType   DataSetType[4];                                       ///<тип данных канала: ток в мкА или приведенный ЦАП
    uint16_t              Data[4];                                              ///<выходное значение, соотвествующее DataSetType с учетом диапазонов в приведенном отсчете ЦАП или токе в мкА
  }Write;    
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_AO4_USE_SIMULATION > 0
    struct{
      uint8_t   DataSetType[4];
      uint8_t   Data[4];
    }Simulation;
  #endif
}K19_AO4_str;


///типы выходных сигналов
#define UL_K15_DO8_TYPE_OUTPUT        0
#define UL_K15_DO8_TYPE_GENERATOR     0xFFFFFFFF

/*macros***********************************************************************/
///отправка PDO Transmit
#define UL_K15_DO8_WRITE_OUT_STATES(index)   UL_PDOTransmit(_K15_DO8_, index, _K15_DO8_PDO_)

/*enums************************************************************************/
///каналы K15_DO8
typedef enum { 
  UL_K15_DO8_Ch1, UL_K15_DO8_Ch2, UL_K15_DO8_Ch3, UL_K15_DO8_Ch4,
  UL_K15_DO8_Ch5, UL_K15_DO8_Ch6, UL_K15_DO8_Ch7, UL_K15_DO8_Ch8
}UL_K15_DO8_Channels __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum { 
  UL_K15_DO8_OutLow  = 0,
  UL_K15_DO8_OutHigh = 1 
}UL_K15_DO8_OutState __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K15_DO8
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K15_DO8_OutState    Out[8];                                              ///<считанные значения текущих выходных состояний каналов
    float                  Frequency[8];                                        ///<считанные значения частот каналов.
    uint32_t               Counter[8];                                          ///<считанные значения счетчиков каналов.
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{                                                                       
    UL_K15_DO8_OutState    Out[8];                                              ///<значения выходных состояний каналов
  }Write;    
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_DO8_USE_SIMULATION > 0
    struct{
      uint8_t   Out[8];     
    }Simulation;
  #endif  
}K15_DO8_str;


/*enums************************************************************************/
///каналы DI
typedef enum { 
  UL_K19_DIDO8_DI_CH1 = 0, 
  UL_K19_DIDO8_DI_CH2,
  UL_K19_DIDO8_DI_CH3,
  UL_K19_DIDO8_DI_CH4,
  UL_K19_DIDO8_DI_CH5,
  UL_K19_DIDO8_DI_CH6,
  UL_K19_DIDO8_DI_CH7,
  UL_K19_DIDO8_DI_CH8,
}UL_K19_DIDO8_DI_Channels __attribute__ ((mode(__byte__)));

///каналы DO
typedef enum { 
  UL_K19_DIDO8_DO_CH1 = 0, 
  UL_K19_DIDO8_DO_CH2,
  UL_K19_DIDO8_DO_CH3,
  UL_K19_DIDO8_DO_CH4,
  UL_K19_DIDO8_DO_CH5,
  UL_K19_DIDO8_DO_CH6,
  UL_K19_DIDO8_DO_CH7,
  UL_K19_DIDO8_DO_CH8,
}UL_K19_DIDO8_DO_Channels __attribute__ ((mode(__byte__)));

///состояние канала
typedef enum {
  UL_K19_DIDO8_ChStLow  = 0, 
  UL_K19_DIDO8_ChStHigh = 1 
}UL_K19_DIDO8_ChState __attribute__ ((mode(__byte__)));    

/*structures*******************************************************************/
///структура модуля K19_DIDO8
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    UL_K19_DIDO8_ChState    DI[8];                                              ///<считанные состояния входов каналов DI
    UL_K19_DIDO8_ChState    DO[8];                                              ///<считанные состояния выходов каналов DO
    uint32_t                DICounter[8];                                       ///<считанные значения счетчиков каналов DI
    uint16_t                DIFilter;                                           ///<считанные значения фильтра каналов DI в мс. Работает для всех каналов DI. 10 - 1000 мс. 100 по умолчанию
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{
    uint16_t                DIFilter;                                           ///<значения фильтра каналов DI. 10 - 1000 мс. 100 по умолчанию 
    UL_K19_DIDO8_ChState    DO[8];                                              ///<значения выходов DO для записи
  }Write;
  ///системная информация 
  ModuleSysData_str System;                                                     ///<системная информация
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_DIDO8_USE_SIMULATION > 0
    struct{
      uint8_t   DI[8];
      uint8_t   DO[8];
      uint8_t   DICounter[16];
    }Simulation;
  #endif   
}K19_DIDO8_str;

/*enums************************************************************************/
///каналы 1,2,3,4 - только дискретные входы с счетчиком
///каналы 5,6,7,8 - дискретные входы с счетчиками и измерением частоты
typedef enum { 
  UL_K15_FDI8_DiCh1 = 0,
  UL_K15_FDI8_DiCh2,
  UL_K15_FDI8_DiCh3,
  UL_K15_FDI8_DiCh4,
  UL_K15_FDI8_DiFiCh5,
  UL_K15_FDI8_DiFiCh6,
  UL_K15_FDI8_DiFiCh7,
  UL_K15_FDI8_DiFiCh8
}UL_K15_FDI8_Channels __attribute__ ((mode(__byte__)));

typedef enum { 
  UL_K15_FDI8_DiSt_Low = 0,
  UL_K15_FDI8_DiSt_High
}UL_K15_FDI8_DiState __attribute__ ((mode(__byte__)));


/*structures*******************************************************************/
///структура каналов c дискретнми входами с счетчиками
typedef struct{  
  UL_K15_FDI8_DiState In;                                                       ///<текущее состояние входа
  uint32_t            Counter;                                                  ///<накопленное количество импульсов каналов
  uint16_t            DiFilter;                                                 ///<фильтр входного дискретного сигнала, мс (0 - 2000).
}K15_FDI8_DiCh_t;

///структура каналов c дискретнми входами с счетчиками и измерением частоты
typedef struct{  
  UL_K15_FDI8_DiState In;                                                       ///<текущее состояние входа
  uint32_t            Counter;                                                  ///<накопленное количество импульсов каналов
  uint16_t            DiFilter;                                                 ///<фильтр входного дискретного сигнала, мс (0 - 2000).
  float               Frequency;                                                ///<считанные значения частот в Гц на входах
  /**
     Фильтр частоты каналов. Значение в мс. Отвечает за время усреднения входящей частоты.
     Также определяет минимально достоверную частоту измерения. Например, установив фильтр в 500мс установление достоверного значения частоты
     будет занимать 500мс, но при этом, минимальная частота для измерения составит 1000/500 = 2 Гц. Значения частоты меньше 1000/Filter,
     за исключением нуля, будут недостоверными.
     Принимаемые значения 100 - 65535 мс. По умолчанию имеет значение 1000 мс.
     При минимальном значении фильтра минимально достоверная фиксируемая частота составит 1000/100 = 10 Гц. Но, при этом установление
     достоверного значения состоявит 100мс. Подходит при необходимости быстрого отклика.
     При масимальном значении фильтра минимально достоверная фиксируемая частота составит 1000/65535 = 0.01525 Гц. Но, при этом нужно учитывать, 
     что время установления достоверного значения для канала состоянит 65535 мс.
    */
  uint16_t            FiFilter;                                                 ///<текущий фильтр расчета частоты, мс (100 - 65535)
}K15_FDI8_FiCh_t;

///структура модуля K15_FDI8
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    K15_FDI8_DiCh_t     DiCh[4];                                                ///<каналы 1,2,3,4
    K15_FDI8_FiCh_t     FiCh[4];                                                ///<каналы 5,6,7,8
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K15_FDI8_USE_SIMULATION > 0
    struct{
      uint8_t   Frequency[4];
      uint8_t   Counter[8];
      uint8_t   In[8];
    }Simulation;
  #endif  
}K15_FDI8_str;


/*enums************************************************************************/
///каналы K19_FIO
typedef enum { 
  UL_K19_FIO_InCh1 = 0,
  UL_K19_FIO_InCh2,
  UL_K19_FIO_InCh3,
  UL_K19_FIO_InCh4
}UL_K19_FIO_InChannels __attribute__ ((mode(__byte__)));

///Настройка выхода
typedef enum{
  K19_FIO_OUT_NONE = 0,                                                         ///<на выход не транслируется ни один канал
  K19_FIO_OUT_CH1,                                                              ///<на выход транслируется канал 1
  K19_FIO_OUT_CH2,                                                              ///<на выход транслируется канал 2
  K19_FIO_OUT_CH3,                                                              ///<на выход транслируется канал 3
  K19_FIO_OUT_CH4,                                                              ///<на выход транслируется канал 4
  K19_FIO_OUT_SizeDef = 0xFFFF
}K19_FIO_OutType __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K19_FIO
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    float               Frequency[4];                                           ///<считанные значения частот в Гц на входах
    uint32_t            Counter[4];                                             ///<накопленное количество импульсов каналов
    /**
     Фильтр каналов. Значение в мс. Отвечает за время усреднения входящей частоты.
     Также определяет минимально достоверную частоту измерения. Например, установив фильтр в 500мс установление достоверного значения частоты
     будет занимать 500мс, но при этом, минимальная частота для измерения составит 1000/500 = 2 Гц. Значения частоты меньше 1000/Filter,
     за исключением нуля, будут недостоверными.
     Принимаемые значения 100 - 65535 мс. По умолчанию имеет значение 1000 мс.
     При минимальном значении фильтра минимально достоверная фиксируемая частота составит 1000/100 = 10 Гц. Но, при этом установление
     достоверного значения состоявит 100мс. Подходит при необходимости быстрого отклика.
     При масимальном значении фильтра минимально достоверная фиксируемая частота составит 1000/65535 = 0.01525 Гц. Но, при этом нужно учитывать, 
     что время установления достоверного значения для канала состоянит 65535 мс.
    */
    uint16_t            Filter[4];
    K19_FIO_OutType     OutType;                                                ///<текущая настройка выхода
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///данные автоматической записи. достаточно изменить значение в структуре Write
  struct{                                                                       
    K19_FIO_OutType     OutType;
  }Write;    
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_FIO_USE_SIMULATION > 0
    struct{
      uint8_t   Frequency[4];
      uint8_t   Counter[4];      
    }Simulation;
  #endif  
}K19_FIO_str;


/*enums************************************************************************/
///состояние замера
typedef enum{
  K19_CRM_Meas_Idle = 0,                                                        ///<замер не ведется
  K19_CRM_Meas_WaitForFirst,                                                    ///<ожидание первого фронта
  K19_CRM_Meas_WaitForSecond,                                                   ///<ожидание второго фронта
  K19_CRM_Meas_Finished,                                                        ///<замер завершен
  K19_CRM_Meas_Timeout                                                          ///<замер прекращен, так как время ожидания превысило 120 минут
}K19_CRM_MeasureState __attribute__ ((mode(__byte__)));

///команды модуля
typedef enum{
  K19_CRM_Command_Idle = 0,                                                     ///<нет команды
  K19_CRM_Command_StartMeasure,                                                 ///<начать замер
  K19_CRM_Command_StopMeasure                                                   ///<остановить замер
}K19_CRM_Commands __attribute__ ((mode(__byte__)));

/*structures*******************************************************************/
///структура модуля K19_CRM
typedef struct{  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    K19_CRM_Commands      Command;                                              ///<регистр команд модуля
    K19_CRM_MeasureState  MeasureState[2];                                      ///<состояние измерения. [0] для детекторов 1,3. [1] для детекторов 2,4
    float                 MeasureTimeUs[2];                                     ///<время замера в мкс между срабатываниями детекторов. [0] для детекторов 1,3. [1] для детекторов 2,4
    float                 MeasureImpulseNmb[2];                                 ///<количество дробных импульсов между срабатываниями детекторов. [0] для детекторов 1,3. [1] для детекторов 2,4
    float                 Frequency;                                            ///<считанное значение частоты в Гц на входе
    struct{                                                                     ///<статусы детекторов
      unsigned            Ch1Stat : 1;                                          ///<детектор 1 сработал
      unsigned            Ch2Stat : 1;
      unsigned            Ch3Stat : 1;
      unsigned            Ch4Stat : 1;
    }DIStat;
    /**
     Фильтр входного канала. Значение в мс. Отвечает за время усреднения входящей частоты.
     Также определяет минимально достоверную частоту измерения. Например, установив фильтр в 500мс установление достоверного значения частоты
     будет занимать 500мс, но при этом, минимальная частота для измерения составит 1000/500 = 2 Гц. Значения частоты меньше 1000/Filter,
     за исключением нуля, будут недостоверными.
     Принимаемые значения 100 - 65535 мс. По умолчанию имеет значение 2000 мс.
     При минимальном значении фильтра минимально достоверная фиксируемая частота составит 1000/100 = 10 Гц. Но, при этом установление
     достоверного значения состоявит 100мс. Подходит при необходимости быстрого отклика.
     При масимальном значении фильтра минимально достоверная фиксируемая частота составит 1000/65535 = 0.01525 Гц. Но, при этом нужно учитывать, 
     что время установления достоверного значения для канала состоянит 65535 мс.
    */
    uint16_t            Filter;
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///системная информация 
  ModuleSysData_str System;
  ///флаги симуляции значений. выставив 1 в нужный канал, данные в Read перестанут обновляться
  #if UL_K19_CRM_USE_SIMULATION > 0
    struct{
      uint8_t   MeasureState[2];
      uint8_t   MeasureTimeUs[2];
      uint8_t   MeasureImpulseNmb[2];
      uint8_t   Frequency;
    }Simulation;
  #endif  
}K19_CRM_str;

/*enums************************************************************************/
///номер канала приема/передачи
typedef enum {
  K19_SCM_Ch1 = 0,
  K19_SCM_Ch2,
  K19_SCM_Ch3,
  K19_SCM_Ch4,
  K19_SCM_Ch_SizeDef = 0xFFFF
} K19_SCM_Channel_t __attribute__ ((mode(__byte__)));

///тип интерфейса приема/передачи
typedef enum {
  K19_SCM_COMT_485only = 0,                                                     ///<текущий интерфейс порта - RS485. Без возможности изменения
  K19_SCM_COMT_485,                                                             ///<текущий интерфейс порта - RS485. Возможно изменить на RS232
  K19_SCM_COMT_232,                                                              ///<текущий интерфейс порта - RS232. Возможно изменить на RS485
  K19_SCM_COMT_SizeDef = 0xFFFF
} K19_SCM_COMType __attribute__ ((mode(__byte__)));

///коды скорости передачи данных канала, бод/с
typedef enum {
  K19_SCM_BR_9600 = 0,
  K19_SCM_BR_14400,
  K19_SCM_BR_19200,
  K19_SCM_BR_28800,
  K19_SCM_BR_38400,
  K19_SCM_BR_56000,
  K19_SCM_BR_57600,
  K19_SCM_BR_115200,
  K19_SCM_BR_Undef = 0x0F,
  K19_SCM_BR_SizeDef = 0xFFFF
} K19_SCM_Baudrate __attribute__ ((mode(__byte__)));

///коды количества стопбит канала
typedef enum {
  K19_SCM_SB_1 = 0,
  K19_SCM_SB_2,
  K19_SCM_SB_SizeDef = 0xFFFF
} K19_SCM_StopBits __attribute__ ((mode(__byte__)));

///коды четности канала
typedef enum{
  K19_SCM_Parity_None = 0,
  K19_SCM_Parity_Even,
  K19_SCM_Parity_Odd,
  K19_SCM_Parity_SizeDef = 0xFFFF
}K19_SCM_Parity __attribute__ ((mode(__byte__)));

///тип настройки количества хранимых пакетов и максимальной длины
typedef enum{
  K19_SCM_BC_16_64 = 0,                                                         ///<возможно хранение до 16х пакетов по 64 байт
  K19_SCM_BC_8_128,                                                             ///<возможно хранение до 8х пакетов по 128 байт
  K19_SCM_BC_4_256,                                                             ///<возможно хранение до 4х пакетов по 256 байт
  K19_SCM_BC_SizeDef = 0xFFFF
} K19_SCM_BuffCfg __attribute__ ((mode(__byte__)));

///коды ошибок функций приема пакета Rx и пакета передачи Tx
typedef enum {
  K19_SCM_ER_Success = 0,                                                       ///<нет ошибок
  K19_SCM_ER_WrongKey,                                                          ///<неверный ключ пакета
  K19_SCM_ER_WrongIndex,                                                        ///<неверный индекс пакета
  K19_SCM_ER_BufferEmpty,                                                       ///<буфер Rx пуст, передавать нечего
  K19_SCM_ER_BufferFull,                                                        ///<буфер Tx полный, некуда складывать пакет        
  K19_SCM_ER_WrongStatus,                                                       ///<неверное состояние заголовка обмена данными
  K19_SCM_ER_WrongPart,                                                         ///<задана/принята неверная часть пакета
  K19_SCM_ER_WrongLen,                                                          ///<задана/принята неверная длина пакета
  K19_SCM_ER_WrongCh,                                                           ///<задан/принят неверный канал 
  K19_SCM_ER_NotDefined,                                                         ///<необрабатываемое состояние
  K19_SCM_ER_SizeDef = 0xFFFF
} K19_SCM_ExchangeResult __attribute__ ((mode(__byte__)));

///структура записи настроек канала
typedef struct {
  K19_SCM_Channel_t Ch;                                                         ///<Канал передачи данных
  K19_SCM_Baudrate  Baudrate;                                                   ///<Скорость передачи данных канала типа K19_SCM_Baudrate
  K19_SCM_StopBits  StopBits;                                                   ///<Количество стоп бит канала типа K19_SCM_StopBits
  K19_SCM_Parity    Parity;                                                     ///<Четность канала типа K19_SCM_Parity
  K19_SCM_COMType   COMType;                                                    ///<Тип интерфейса канала типа K19_SCM_COMType.
                                                                                ///<Для каналов 1, 2 данный параметр не имеет значения, так как их тип всегда RS485 (K19_SCM_COMT_485only)
                                                                                ///<Для каналов 3, 4 интерфейс может изменяться между RS232 (K19_SCM_COMT_232) и RS485 (K19_SCM_COMT_485)
  uint8_t           SepDelayMs;                                                 ///<Интервал тишины в мс до завершения приема пакета и интервал тишины между передачами пакетов (5 - 250 мс)
  K19_SCM_BuffCfg   RxbuffCfg;                                                  ///<Конфигурация буфера приема типа K19_SCM_BuffCfg
  K19_SCM_BuffCfg   TxbuffCfg;                                                  ///<Конфигурация буфера передачи типа K19_SCM_BuffCfg
}K19_SCM_WriteSettings_t;


/*structures*******************************************************************/
///структура считываемых настроек и данных канала
typedef struct {
  uint16_t          Baudrate;                                                   ///скорость передачи данных канала, бод/с
  uint16_t          StopBits;                                                   ///количество стопбит канала
  uint16_t          Parity;                                                     ///четность канала
  uint16_t          COMType;                                                    ///тип интерфейса приема/передачи
  uint16_t          SepDelayMs;                                                 ///<интервал тишины, мс. Время ожидание до завершения приема пакета и задержка между отправкими пакетов
  uint16_t          RxPacketsPending;                                           ///<количество пакетов, ожидающих считывания
  uint16_t          RxBuffCfg;                                                  ///<конфигурация буфера приема
  uint16_t          TxPacketsPending;                                           ///<количество пакетов, переданных на модуль, но еще не отправленных
  uint16_t          TxBuffCfg;                                                  ///<конфигурация буфера передачи
} K19_SCM_Ch_t;

///структура модуля K19_SCM
typedef struct {  
  ///информация о номере модуля и рабочем состоянии
  ModuleInfo_str ModuleInfo;
  ///прочитанные данные
  struct{
    K19_SCM_Ch_t        Ch[4];                                                  ///<каналы приема/передачи
    struct{                                                                     ///<статусы
      uint16_t  CAN, Firmware, Hardware;    
    }Status; 
  }Read;
  ///системная информация 
  ModuleSysData_str System;
} K19_SCM_str;

#endif

typedef enum {
  _M0 = 0,
  _M1,
  _M2,
  _M3,
  _M4,
  _M5,
  _M6,
  _M7
} k15do16_e __attribute__ ((mode(__byte__)));

typedef struct {
    uint16_t                  ch;
    uint16_t                  enable;
    uint16_t                  type;
    uint16_t                  state;
    uint16_t                  firstDotUA;
    uint16_t                  secondDotUA;
    uint16_t                  diFilter;
    uint16_t                  fiFilter;
    uint16_t                  filter;
    uint8_t                   dot;
    float                     frequency;
    float                     out;
    uint32_t                  data;
} KXX_str;

typedef struct {
    uint8_t                   Ch;
    uint16_t                  len;
    K19_SCM_ExchangeResult    exchangeResult;
    uint8_t                   overflowFl;
    uint8_t                   data[256];
} K19_SCMex_str;

#define K19_SCMset_str K19_SCM_WriteSettings_t

typedef struct // the_message
{
  
  struct head_s {
    uint32_t    packet_id;          // first 128 bit is for sign
    uint32_t    m_data_len;         // module data len
    uint64_t    packet_counter;
    uint32_t    m_func;
    uint32_t    m_status;           // additional address
    uint32_t    m_idx;              // module index 0..7
    uint32_t    m_addr;             // additional address
  } head;  
  
  union {
  
    KXX_str           kxx_str;
    K19_SCMex_str     scm_exchange_str;
    K19_SCMset_str    scm_settings_str;

    K15_DO16_str      k15do16_str;
    K15_DI16_str      k15di16_str;
    K15_AI8_str       k15ai8_str;
    K15_AO2_str       k15ao2_str;
    K15_AI6_str       k15ai6_str;
    K15_DO8pwr_str    k15do8pwr_str;
    K15_NAM_str       k15nam_str;
    
    K19_AI6p_str      k19ai6p_str;
    K19_AI6a_str      k19ai6a_str;
    K19_DI16_str      k19di16_str;
    K19_DO16_str      k19do16_str;
    K19_AO4_str       k19ao4_str;

    K15_DO8_str       k15do8_str;
    K19_DIDO8_str     k19dido8_str;
    K15_FDI8_str      k15fdi8_str;
    
    K19_FIO_str       k19fio_str;
    K19_CRM_str       k19crm_str;
    K19_SCM_str       k19scm_str;
    
    uint8_t           DATA8[256];
    uint16_t          DATA16[128];
    uint32_t          DATA32[64];
  } data;
  
  uint8_t     tail[16];
  
} THE_MESSAGE, * THE_MESSAGE_PTR;

#define RPMSG_MESSAGE_MAX_SIZE            (sizeof(THE_MESSAGE)-sizeof(uint8_t)*16)


/*
typedef struct message_processor_t {
  int             func;
  int             func_type;
  void            *func_addr;
  void            *arg[10];
};

UL_RESULT (call_type_1 *) (uint8_t);
UL_RESULT (call_type_2 *) (uint8_t, uint8_t);
UL_RESULT (call_type_3 *) (uint8_t, uint8_t, uint8_t);
UL_RESULT (call_type_4 *) (uint8_t, uint8_t, uint8_t, uint8_t);
UL_RESULT (call_type_5 *) (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

UL_RESULT (call_type_10 *) (uint8_t, int);
UL_RESULT (call_type_11 *) (uint8_t, int, uint8_t);
UL_RESULT (call_type_12 *) (uint8_t, int, uint8_t, uint8_t);

UL_RESULT (call_type_20 *) (uint8_t, int, float, uint32_t, int);

typedef enum {
  CALL_TYPE_1     = 1,
  CALL_TYPE_2,
  CALL_TYPE_3,
  CALL_TYPE_4,
  CALL_TYPE_5,
  CALL_TYPE_10    = 10,
  CALL_TYPE_11,
  CALL_TYPE_12,
  CALL_TYPE_20    = 20,
} func_type_t;

message_processor_t msg_proc = {
  {  _K15_DO16_ChSet, CALL_TYPE_20, &K15_DO16_ChSet, { 0 }  },
  {  _K15_DO16_ReadData, CALL_TYPE_1, &K15_DO16_ReadData, { 0 }  },
  {  _K15_DO16_WriteData, CALL_TYPE_1, &K15_DO16_WriteData, { 0 }  },
  {  _K15_DO16_Check, CALL_TYPE_1, &K15_DO16_Check, { 0 }  },
};
*/

/*
UL_RESULT K15_DI16_ClearCounter(uint8_t strIndex, UL_K15_DI16_Channels ch);     ///<сброс счетчика
UL_RESULT K15_DI16_ReceiveCounter(uint8_t strIndex, UL_K15_DI16_Channels ch);   ///<принудительное обновление значения счетчика канала
UL_RESULT K15_DI16_ReadData(uint8_t strIndex);                                  ///<досрочное чтение параметров
UL_RESULT K15_DI16_WriteData(uint8_t strIndex);                                 ///<досрочная запись параметров
UL_RESULT K15_DI16_Check(uint8_t strIndex);                                     ///<проверка доступности модуля
*/

typedef enum {

  _K15_DI16_ModuleReadAll         = DevTypeK15_DI16+1,
  _K15_DI16_ModuleExchange,
  _K15_DI16_ClearCounter,
  _K15_DI16_ReceiveCounter,
  _K15_DI16_ReadData,
  _K15_DI16_WriteData,
  
  _K15_DO16_ModuleReadAll         = DevTypeK15_DO16+1,
  _K15_DO16_ModuleExchange,
  _K15_DO16_ChSet,
  _K15_DO16_ReadData,
  _K15_DO16_WriteData,

  _K15_AI8_ModuleReadAll          = DevTypeK15_AI8+1,
  _K15_AI8_ModuleExchange,
  _K15_AI8_ADC_Enable,
  _K15_AI8_ADC_Disable,

  _K15_AO2_ModuleReadAll          = DevTypeK15_AO2+1,
  _K15_AO2_ModuleExchange,
  _K15_AO2_DAC1_control,
  _K15_AO2_DAC2_control,
  _K15_AO2_WriteData,
  
  _K15_AI6_ModuleReadAll          = DevTypeK15_AI6+1,
  _K15_AI6_ModuleExchange,
  _K15_AI6_ADC_Enable,
  _K15_AI6_ADC_Disable,
  
  _K15_DO8pwr_ModuleReadAll      = DevTypeK15_DO8pwr+1,
  _K15_DO8pwr_ModuleExchange,
  _K15_DO8pwr_ChSet,
  _K15_DO8pwr_ReadData,
  _K15_DO8pwr_WriteData,

  _K15_NAM_ModuleReadAll          = DevTypeK15_NAMUR+1,
  _K15_NAM_ModuleExchange,
  _K15_NAM_WriteData,
  
  _K19_AI6p_ModuleReadAll         = DevTypeK19_AI6p+1,
  _K19_AI6p_ModuleExchange,
  _K19_AI6p_CorrSave,
  _K19_AI6p_CorrClear,
  _K19_AI6p_CorrSetDot,
  
  _K19_AI6a_ModuleReadAll         = DevTypeK19_AI6a+1,
  _K19_AI6a_ModuleExchange,
  _K19_AI6a_CorrSave,
  _K19_AI6a_CorrClear,
  _K19_AI6a_CorrSetDot,
  
  _K19_DI16_ModuleReadAll         = DevTypeK19_DI16+1,
  _K19_DI16_ModuleExchange,
  _K19_DI16_ClearCounter,
  _K19_DI16_ReceiveCounter,
  _K19_DI16_ReadData,
  _K19_DI16_WriteData,
  
  _K19_DO16_ModuleReadAll         = DevTypeK19_DO16+1,
  _K19_DO16_ModuleExchange,
  _K19_DO16_ReadData,
  _K19_DO16_WriteData,
  
  _K19_AO4_ModuleReadAll          = DevTypeK19_AO4+1,
  _K19_AO4_ModuleExchange,
  _K19_AO4_ReadData,
  _K19_AO4_WriteData,
  _K19_AO4_CalibrationHandler,

  _K15_DO8_ModuleReadAll          = DevTypeK15_DO8+1,
  _K15_DO8_ModuleExchange,
  _K15_DO8_ChSet,
  _K15_DO8_ReadData,
  _K15_DO8_WriteData,

  _K19_DIDO8_ModuleReadAll        = DevTypeK19_DIDO8+1,
  _K19_DIDO8_ModuleExchange,
  _K19_DIDO8_ClearCounter,
  _K19_DIDO8_ReceiveCounter,
  _K19_DIDO8_ReadData,
  _K19_DIDO8_WriteData,
  
  _K15_FDI8_ModuleReadAll         = DevTypeK15_FDI8+1,
  _K15_FDI8_ModuleExchange,
  _K15_FDI8_ClearCounter,
  _K15_FDI8_SetDiFilter,
  _K15_FDI8_SetFiFilter,
  _K15_FDI8_ReadData,
  _K15_FDI8_WriteData,
  
  _K19_FIO_ModuleReadAll          = DevTypeK19_FIO+1,
  _K19_FIO_ModuleExchange,
  _K19_FIO_ClearCounter,
  _K19_FIO_SetFilter,
  _K19_FIO_ReadData,
  _K19_FIO_WriteData,

  _K19_CRM_ModuleReadAll          = DevTypeK19_CRM+1,
  _K19_CRM_ModuleExchange,
  _K19_CRM_SetFilter,
  _K19_CRM_MeasureStart,
  _K19_CRM_MeasureStop,

  _K19_SCM_ModuleReadAll          = DevTypeK19_SCM+1,
  _K19_SCM_ModuleExchange,
  _K19_SCM_ReadPacket,
  _K19_SCM_WritePacket,
  _K19_SCM_WriteSettings,
  

} ModulesFunc __attribute__ ((mode(__byte__)));

#pragma pack(pop)

#endif



