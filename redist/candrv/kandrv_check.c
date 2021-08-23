/*
========
  HELP
========
pre-execute command in the command line:
> - ip link set can0 up type can bitrate 1000000
> - ip link set can0 down
> - ifconfig can0 up
> - ifconfig can0 down
> - ip -d link show can0
> - candump -t A can0
> - ip -details -statistics link show can0

> compile and build image:
> bitbake -c menuconfig virtual/kernel
> bitbake fsl-image-gui

to compile:
> sudo apt-get install gcc-arm*
> bitbake -c compile -f kandrv
> ls ./tmp/work-shared/imx7-var-som/kernel-source/drivers/rpmsg/

  
  ssh-keygen -f "/home/anton/.ssh/known_hosts" -R "192.168.1.101"
  scp ./tmp/work/cortexa7t2hf-neon-fslc-linux-gnueabi/kandrv/0.1-r0/kandrv_check root@192.168.1.101:/usr/bin/kandrv_check

  zcat ./tmp/deploy/images/imx7-var-som/fsl-image-gui-imx7-var-som.wic.gz | sudo dd of=/dev/sdc bs=1M status=progress && sync

  USE -fshort-enums flag to build headers

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <sched.h>

#include "rpmsg_connector.h"

// TERMINATE (CTRL+C)
volatile sig_atomic_t terminate_flag = 0;
void _signal_sigint(int sig) { terminate_flag = 1; }

K15_DO16_str    k15do16[8];
K15_DI16_str    k15di16[8];
K15_AI8_str     k15ai8[8];
K15_AO2_str     k15ao2[8];
K15_AI6_str     k15ai6[8];
K15_DO8_str     k15do8[8];
K15_NAM_str     k15nam[8];
K19_AI6p_str    k19ai6p[8];
K19_AI6p_str    k19ai6a[8];
K19_DI16_str    k19di16[8];
K19_DO16_str    k19do16[8];
K19_AO4_str     k19ao4[8];
K15_DO8pwr_str  k15do8pwr[8];

K19_DIDO8_str   k19dido8[8];
K15_FDI8_str    k15dfi8[8];
K19_FIO_str     k19fio[8];
K19_CRM_str     k19crm[8];
K19_SCM_str     k19scm[8];

KXX_str         Kxx_Settings;
K19_SCMex_str   K19_SCM_exchange, K19_SCM_exchange_copy;
K19_SCMset_str  K19_SCM_settings;


K19_SCM_Ch_t ch_test;

void printf_bin(UL_K15_DI16_ChState *data, int size);
void printf_bin(UL_K15_DI16_ChState *data, int size) {
  int i;
  
  for(i=0; i<size; i++, data++) {
    printf("%d", *data);
  }
}
void printf_float(float *data, int size);
void printf_float(float *data, int size) {
  int i;
  for(i=0; i<size; i++, data++) {
    printf("ch[%d] = %.3f ", i, *data);
  }
  
}

void printf_uint16(uint16_t *data, int size);
void printf_uint16(uint16_t *data, int size) {
  int i;
  for(i=0; i<size; i++, data++) {
    printf("ch[%d] = %d ", i, *data);
  }
  
}

void printf_uint8(uint8_t *data, int size);
void printf_uint8(uint8_t *data, int size) {
  int i;
  for(i=0; i<size; i++, data++) {
    printf("ch[%d] = %d ", i, *data);
  }
  
}


int main(int argc, char **argv) {
  int   flag = 1;
  int   i,j, scm_stage = 0, scm_tries = 0;
  
  float CrmFrequency = 0.0;
  //struct sched_param          schedparm;

  //memset(&schedparm, 0, sizeof(schedparm));
  //schedparm.sched_priority = 21;
  //sched_setscheduler(0, SCHED_FIFO, &schedparm);

  signal(SIGINT, _signal_sigint);
  
  //candrv_printf("Start CAN Driver checker\n");
  //candrv_printf("Device name: %s\n", can_name);
  //candrv_t *conn = candrv_init(&can_name[0]);
  
  int cnt = 0;
  terminate_flag = 0;
  
  // candrv_lss_init_modules(conn);
  
  int res = candrv_init();

  int sw  = 0;  
  int sw_out  = 0;  


  printf("sizeof(UL_K15_DO16_OutState) = %d\n\r", sizeof(UL_K15_DO16_OutState));
  printf("sizeof(int) = %d\n\r", sizeof(int));

  printf("****** offset(0): %d\n\r", (uint32_t)&k15do16[0].Write - (uint32_t)&k15do16[0]);
  printf("****** offset(1): %d\n\r", (uint32_t)&k15do16[0].Write.Out[0] - (uint32_t)&k15do16[0]);

  printf("\n\r Size of K19_SCM_str : %d ", sizeof(K19_SCM_str));
  printf("\n\r Size of K19_SCM_Ch_t : %d ", sizeof(K19_SCM_Ch_t));
  
  printf("\n\r Size of ch_test.Baudrate : %d ", sizeof(ch_test.Baudrate));
  printf("\n\r Size of ch_test.StopBits : %d ", sizeof(ch_test.StopBits));
  printf("\n\r Size of ch_test.Parity : %d ", sizeof(ch_test.Parity));
  printf("\n\r Size of ch_test.COMType : %d ", sizeof(ch_test.COMType));
  printf("\n\r Size of ch_test.SepDelayMs : %d ", sizeof(ch_test.SepDelayMs));
  printf("\n\r Size of ch_test.RxPacketsPending : %d ", sizeof(ch_test.RxPacketsPending));
  printf("\n\r Size of ch_test.RxBuffCfg : %d ", sizeof(ch_test.RxBuffCfg));
  printf("\n\r Size of ch_test.TxPacketsPending : %d ", sizeof(ch_test.TxPacketsPending));
  printf("\n\r Size of ch_test.TxBuffCfg : %d ", sizeof(ch_test.TxBuffCfg));
  printf("\n\r ");

  // return;

  // usleep(1000);
  
  // init modules
  
  memset( &k15do16[0], 0, sizeof(k15do16) );
  memset( &k15di16[0], 0, sizeof(k15di16) );
  memset( &k15ai8[0], 0, sizeof(k15ai8) );
  memset( &k15ao2[0], 0, sizeof(k15ao2) );
  memset( &k15ai6[0], 0, sizeof(k15ai6) );
  memset( &k15do8[0], 0, sizeof(k15do8) );
  memset( &k15nam[0], 0, sizeof(k15nam) );

  memset( &k19ai6p[0], 0, sizeof(k19ai6p) );
  memset( &k19ai6a[0], 0, sizeof(k19ai6a) );
  memset( &k19di16[0], 0, sizeof(k19di16) );
  memset( &k19do16[0], 0, sizeof(k19do16) );
  memset( &k19ao4[0], 0, sizeof(k19ao4) );

  memset( &k15do8pwr[0], 0, sizeof(k15do8pwr) );
  memset( &k19dido8[0], 0, sizeof(k19dido8) );
  memset( &k15dfi8[0], 0, sizeof(k15dfi8) );
  memset( &k19fio[0], 0, sizeof(k19fio) );
  memset( &k19crm[0], 0, sizeof(k19crm) );

  memset( &k19scm[0], 0, sizeof(k19scm) );
  memset( &K19_SCM_exchange, 0, sizeof(K19_SCM_exchange) );
  memset( &K19_SCM_exchange_copy, 0, sizeof(K19_SCM_exchange_copy) );



  while(flag) {
    // printf("*");
    // check is module is available
    if( !k15do16[0].ModuleInfo.InWork ) candrv_cmd( _K15_DO16_ModuleReadAll, _M0, &k15do16[0] );
    if(  k15do16[0].ModuleInfo.InWork ) candrv_cmd( _K15_DO16_ModuleExchange, _M0, &k15do16[0] );

// printf("1\n\r");
    
    // check is module is available
    if( !k15di16[0].ModuleInfo.InWork ) candrv_cmd( _K15_DI16_ModuleReadAll, _M0, &k15di16[0] );
    if(  k15di16[0].ModuleInfo.InWork ) candrv_cmd( _K15_DI16_ModuleExchange, _M0, &k15di16[0] );

// printf("2\n\r");
    
    // check is module is available
    if( !k15ai8[0].ModuleInfo.InWork ) candrv_cmd( _K15_AI8_ModuleReadAll, _M0, &k15ai8[0] );
    if(  k15ai8[0].ModuleInfo.InWork ) candrv_cmd( _K15_AI8_ModuleExchange, _M0, &k15ai8[0] );

// printf("3\n\r");
    
    // check is module is available
    if( !k15ao2[0].ModuleInfo.InWork ) candrv_cmd( _K15_AO2_ModuleReadAll, _M0, &k15ao2[0] );
    if(  k15ao2[0].ModuleInfo.InWork ) {
      for(i=0; i<2; i++) {
        k15ao2[0].Write.Enable[i] = UL_K15_AO2_Enable;
        k15ao2[0].Write.Source[i] = UL_K15_AO2_Current;
      }
      candrv_cmd( _K15_AO2_ModuleExchange, _M0, &k15ao2[0] );
    }

// printf("4\n\r");

    // check is module is available
    if( !k15ai6[0].ModuleInfo.InWork ) candrv_cmd( _K15_AI6_ModuleReadAll, _M0, &k15ai6[0] );
    if(  k15ai6[0].ModuleInfo.InWork ) candrv_cmd( _K15_AI6_ModuleExchange, _M0, &k15ai6[0] );

// printf("5\n\r");
 
    if( !k15do8[0].ModuleInfo.InWork ) candrv_cmd( _K15_DO8_ModuleReadAll, _M0, &k15do8[0] );
    if(  k15do8[0].ModuleInfo.InWork ) candrv_cmd( _K15_DO8_ModuleExchange, _M0, &k15do8[0] );

// printf("6\n\r");

    if( !k15nam[0].ModuleInfo.InWork ) candrv_cmd( _K15_NAM_ModuleReadAll, _M0, &k15nam[0] );
    if(  k15nam[0].ModuleInfo.InWork ) candrv_cmd( _K15_NAM_ModuleExchange, _M0, &k15nam[0] );

// printf("7\n\r");

    if( !k19ai6p[0].ModuleInfo.InWork ) candrv_cmd( _K19_AI6p_ModuleReadAll, _M0, &k19ai6p[0] );
    if(  k19ai6p[0].ModuleInfo.InWork ) candrv_cmd( _K19_AI6p_ModuleExchange, _M0, &k19ai6p[0] );

// printf("8\n\r");

    if( !k19ai6a[0].ModuleInfo.InWork ) candrv_cmd( _K19_AI6a_ModuleReadAll, _M0, &k19ai6a[0] );
    if(  k19ai6a[0].ModuleInfo.InWork ) candrv_cmd( _K19_AI6a_ModuleExchange, _M0, &k19ai6a[0] );

// printf("9\n\r");

    if( !k19di16[0].ModuleInfo.InWork ) candrv_cmd( _K19_DI16_ModuleReadAll, _M0, &k19di16[0] );
    if(  k19di16[0].ModuleInfo.InWork ) candrv_cmd( _K19_DI16_ModuleExchange, _M0, &k19di16[0] );

// printf("10\n\r");

    if( !k19do16[0].ModuleInfo.InWork ) candrv_cmd( _K19_DO16_ModuleReadAll, _M0, &k19do16[0] );
    if(  k19do16[0].ModuleInfo.InWork ) candrv_cmd( _K19_DO16_ModuleExchange, _M0, &k19do16[0] );

// printf("11\n\r");

    if( !k19ao4[0].ModuleInfo.InWork ) candrv_cmd( _K19_AO4_ModuleReadAll, _M0, &k19ao4[0] );
    if(  k19ao4[0].ModuleInfo.InWork ) {
      for(i=0; i<4; i++) { 
        k19ao4[0].Write.DataSetType[i] = K19_AO4_DST_TrueUA;
      }
      candrv_cmd( _K19_AO4_ModuleExchange, _M0, &k19ao4[0] );
    }

// printf("11\n\r");

    if( !k15do8pwr[0].ModuleInfo.InWork ) candrv_cmd( _K15_DO8pwr_ModuleReadAll, _M0, &k15do8pwr[0] );
    if(  k15do8pwr[0].ModuleInfo.InWork ) candrv_cmd( _K15_DO8pwr_ModuleExchange, _M0, &k15do8pwr[0] );

// printf("12\n\r");

/*

+ K19_DIDO8_str   k19dido8[8];
+ K15_FDI8_str    k15dfi8[8];
+ K19_FIO_str     k19fio[8];
+ K19_CRM_str     k19crm[8];
+ K19_SCM_str     k19scm[8];

KXX_str         Kxx_Settings;
K19_SCMex_str   K19_SCM_exchange;
K19_SCMset_str  K19_SCM_settings;

*/

    if( !k19dido8[0].ModuleInfo.InWork ) candrv_cmd( _K19_DIDO8_ModuleReadAll, _M0, &k19dido8[0] );
    if(  k19dido8[0].ModuleInfo.InWork ) candrv_cmd( _K19_DIDO8_ModuleExchange, _M0, &k19dido8[0] );

// printf("13\n\r");

    if( !k15dfi8[0].ModuleInfo.InWork ) candrv_cmd( _K15_FDI8_ModuleReadAll, _M0, &k15dfi8[0] );
    if(  k15dfi8[0].ModuleInfo.InWork ) candrv_cmd( _K15_FDI8_ModuleExchange, _M0, &k15dfi8[0] );

// printf("14\n\r");

    if( !k19fio[0].ModuleInfo.InWork ) candrv_cmd( _K19_FIO_ModuleReadAll, _M0, &k19fio[0] );
    if(  k19fio[0].ModuleInfo.InWork ) candrv_cmd( _K19_FIO_ModuleExchange, _M0, &k19fio[0] );

// printf("15\n\r");

    if( !k19crm[0].ModuleInfo.InWork ) candrv_cmd( _K19_CRM_ModuleReadAll, _M0, &k19crm[0] );
    if(  k19crm[0].ModuleInfo.InWork ) {
      candrv_cmd( _K19_CRM_ModuleExchange, _M0, &k19crm[0] ); 
      if((k19crm[0].Read.Command != K19_CRM_Meas_WaitForFirst) && (k19crm[0].Read.Command != K19_CRM_Meas_WaitForSecond)) {
        candrv_cmd( _K19_CRM_MeasureStart, _M0, NULL );
        CrmFrequency = k19crm[0].Read.Frequency;
      }
    }
    
    /*
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
        } K19_SCM_WriteSettings_t;    
    */
    
// printf("16\n\r");

    if( !k19scm[0].ModuleInfo.InWork ) {
      candrv_cmd( _K19_SCM_ModuleReadAll, _M0, &k19scm[0] );
      scm_stage = 0;
    }

// printf("17\n\r");

    if(  k19scm[0].ModuleInfo.InWork ) { 
      switch(scm_stage) {
        case 0:
          K19_SCM_settings.Ch         = K19_SCM_Ch1;
          K19_SCM_settings.Baudrate   = K19_SCM_BR_9600;
          K19_SCM_settings.StopBits   = K19_SCM_SB_1;
          K19_SCM_settings.Parity     = K19_SCM_Parity_None;
          K19_SCM_settings.COMType    = K19_SCM_COMT_485only;
          K19_SCM_settings.SepDelayMs = 50;
          K19_SCM_settings.RxbuffCfg  = K19_SCM_BC_8_128;
          K19_SCM_settings.TxbuffCfg  = K19_SCM_BC_8_128;
          candrv_cmd( _K19_SCM_WriteSettings, _M0, &K19_SCM_settings );
          
          K19_SCM_settings.Ch         = K19_SCM_Ch2;
          K19_SCM_settings.Baudrate   = K19_SCM_BR_9600;
          K19_SCM_settings.StopBits   = K19_SCM_SB_1;
          K19_SCM_settings.Parity     = K19_SCM_Parity_None;
          K19_SCM_settings.COMType    = K19_SCM_COMT_485only;
          K19_SCM_settings.SepDelayMs = 10;
          K19_SCM_settings.RxbuffCfg  = K19_SCM_BC_8_128;
          K19_SCM_settings.TxbuffCfg  = K19_SCM_BC_8_128;
          candrv_cmd( _K19_SCM_WriteSettings, _M0, &K19_SCM_settings );
          
          scm_stage++;
        break;
        case 1:
          K19_SCM_exchange.Ch         = K19_SCM_Ch1;
          K19_SCM_exchange.data[0]    = 0x44;
          K19_SCM_exchange.data[1]    = 0x33;
          K19_SCM_exchange.data[2]    = 0x22;
          K19_SCM_exchange.data[3]    = 0x11;
          K19_SCM_exchange.len        = 4;
          candrv_cmd( _K19_SCM_WritePacket, _M0, &K19_SCM_exchange );
        break;
        
        case 2:
          memset( &K19_SCM_exchange.data[0], 0, sizeof(K19_SCM_exchange) );
          scm_stage++;
          scm_tries = 0;
        break;
        
        case 3:
          K19_SCM_exchange.Ch         = K19_SCM_Ch2;
          candrv_cmd( _K19_SCM_ReadPacket, _M0, &K19_SCM_exchange );
          scm_tries++;
          if(K19_SCM_exchange.len > 0) {
            memcpy( &K19_SCM_exchange_copy, &K19_SCM_exchange, sizeof(K19_SCM_exchange_copy) );
          }
          if(scm_tries > 5 || K19_SCM_exchange.len > 0 ) scm_stage = 1;
          scm_tries++;
        break;
        
      }
      candrv_cmd( _K19_SCM_ModuleExchange, _M0, &k19scm[0] );
    }

// printf("18\n\r");

    if(terminate_flag) {
      // candrv_printf("\n\rCTRL+C termination\n\r");
      flag = 0;
      break;
    }

    if( sw == 5 ) {
      for(i=0; i<16; i++) k15do16[0].Write.Out[i]     = 1;
      for(i=0; i<8; i++)  k15do8[0].Write.Out[i]      = 1;
      for(i=0; i<8; i++)  k15do8pwr[0].Write.Out[i]   = 1;
      for(i=0; i<16; i++) k19do16[0].Write.Out[i]     = 1;
      for(i=0; i<8; i++)  k19dido8[0].Write.DO[i]     = 1;
      // printf("1");
    }
    else if( sw > 10 ) {
      for(i=0; i<16; i++) k15do16[0].Write.Out[i]     = 0;
      for(i=0; i<8; i++)  k15do8[0].Write.Out[i]      = 0;
      for(i=0; i<8; i++)  k15do8pwr[0].Write.Out[i]   = 0;
      for(i=0; i<16; i++) k19do16[0].Write.Out[i]     = 0;
      for(i=0; i<8; i++)  k19dido8[0].Write.DO[i]     = 0;
      sw = 0;
      // printf("0");
      
      
    }
    
    sw++;
    sw_out++;
    
    // usleep(1000);

    
    if(sw_out > 5) {

      for(i=0; i<2; i++) {
        k15ao2[0].Write.Out[i] += 4.0;
        if(k15ao2[0].Write.Out[i] > 20.0) k15ao2[0].Write.Out[i] = 0.0;
      }
      
      for(i=0; i<4; i++) {
        k19ao4[0].Write.Data[i] += 4000;
        if(k19ao4[0].Write.Data[i] > 20000) k19ao4[0].Write.Data[i] = 0;
      }
      
      printf("InWork = %d,  K15 DO16[0]:   ", k15do16[0].ModuleInfo.InWork); printf_bin( (UL_K15_DI16_ChState*)&k15do16[0].Read.Out[0], 16 ); 
      printf(" \n\r");
      printf("InWork = %d,  K15 DI16[0]:   ", k15di16[0].ModuleInfo.InWork); printf_bin( (UL_K15_DI16_ChState*)&k15di16[0].Read.In[0], 16 );
      printf(" \n\r");
      printf("InWork = %d,  K15 AI8[0]:    ", k15ai8[0].ModuleInfo.InWork); printf_float( &k15ai8[0].Read.Data[0], 8 );
      printf(" \n\r");
      printf("InWork = %d,  K15 AO2[0]:    ", k15ao2[0].ModuleInfo.InWork); printf_float( &k15ao2[0].Read.Out[0], 2 );
      printf(" \n\r");
      printf("InWork = %d,  K15 AI6[0]:    ", k15ai6[0].ModuleInfo.InWork); printf_float( &k15ai6[0].Read.Data[0], 6 );
      printf(" \n\r");
      printf("InWork = %d,  K15 DO8[0]:    ", k15do8[0].ModuleInfo.InWork); printf_bin( (UL_K15_DI16_ChState*)&k15do8[0].Read.Out[0], 8 );
      printf(" \n\r");
      printf("InWork = %d,  K15 NAM[0]:    ", k15nam[0].ModuleInfo.InWork); printf_bin( (UL_K15_NAM_ChState*)&k15nam[0].Read.In[0], 4 );
      printf(" Sc: "); printf_bin( (UL_K15_NAM_ChState*)&k15nam[0].Read.Sc[0], 4 );
      printf(" Wb: "); printf_bin( (UL_K15_NAM_ChState*)&k15nam[0].Read.Wb[0], 4 );
      printf(" \n\r");
      printf("InWork = %d,  K19 AI6p[0]:   ", k19ai6p[0].ModuleInfo.InWork); printf_float( &k19ai6p[0].Read.Data[0], 6 );
      printf(" \n\r");
      printf("InWork = %d,  K19 AI6a[0]:   ", k19ai6a[0].ModuleInfo.InWork); printf_float( &k19ai6a[0].Read.Data[0], 6 );
      printf(" \n\r");
      printf("InWork = %d,  K19 DI16[0]:   ", k19di16[0].ModuleInfo.InWork); printf_bin( (UL_K15_DI16_ChState*)&k19di16[0].Read.In[0], 16 );
      printf(" \n\r");
      printf("InWork = %d,  K19 DO16[0]:   ", k19do16[0].ModuleInfo.InWork); printf_bin( (UL_K15_DI16_ChState*)&k19do16[0].Read.Out[0], 16 );
      printf(" \n\r");
      printf("InWork = %d,  K19 AO4[0]:    ", k19ao4[0].ModuleInfo.InWork); printf_uint16( &k19ao4[0].Read.Data[0], 4 );
      printf(" \n\r");
      
      printf("InWork = %d,  K15 DO8pwr[0]: ", k15do8pwr[0].ModuleInfo.InWork); printf_bin( &k15do8pwr[0].Read.Out[0], 8 );
      printf(" \n\r");

      printf("InWork = %d,  K19 DIDO8[0]:  DI : ", k19dido8[0].ModuleInfo.InWork); printf_bin( &k19dido8[0].Read.DI[0], 8 ); printf("  DO : "); printf_bin( &k19dido8[0].Read.DO[0], 8 );
      printf(" \n\r");

      printf("InWork = %d,  K15 FDI[0]:    DI : ", k15dfi8[0].ModuleInfo.InWork); for(i=0;i<4;i++) printf_bin( &k15dfi8[0].Read.DiCh[i].In, 1);
      printf(" \n\r");

      printf("InWork = %d,  K15 FIO[0]:    Counter ch 0 : %d, Frequency ch 0 : %f", k19fio[0].ModuleInfo.InWork, k19fio[0].Read.Counter[0], k19fio[0].Read.Frequency[0]);
      printf(" \n\r");
      
      printf("InWork = %d,  K15 CRM[0]:    Frequency : %f", k19crm[0].ModuleInfo.InWork, CrmFrequency);
      printf(" \n\r");

      printf("InWork = %d,  K15 SCM[0]:    Last received packet: [ ", k19scm[0].ModuleInfo.InWork); printf_uint8( &K19_SCM_exchange_copy.data[0], K19_SCM_exchange_copy.len ); printf(" ]");
      printf(" \n\r"); 
      
      /*

      K19_CRM_str     k19crm[8];
      K19_SCM_str     k19scm[8];

      KXX_str         Kxx_Settings;
      K19_SCMex_str   K19_SCM_exchange;
      K19_SCMset_str  K19_SCM_settings;

      */
      
      printf(" \n\r");
      
      sw_out = 0;
    }
    
  }
  
  candrv_deinit();
  
  return 0;
}

