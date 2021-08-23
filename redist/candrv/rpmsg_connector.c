/*
 * echo_test.c
 *
 *  Created on: Oct 4, 2014
 *      Author: etsam
 */

/*
 * Test application that data integraty of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
 */

#include <stdint.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <linux/rpmsg.h>

#include "rpmsg_connector.h"


#define _payload    THE_MESSAGE

/*
struct _payload {
	unsigned long num;
	unsigned long size;
	char data[];
};
*/

static int charfd = -1, fd = -1, err_cnt;

_payload *i_payload;
_payload *r_payload;

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (512 - RPMSG_HEADER_LEN)
#define PAYLOAD_MIN_SIZE	1
#define PAYLOAD_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 24)
#define NUM_PAYLOADS		(PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

#define RPMSG_BUS_SYS "/sys/bus/rpmsg"

static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
	int ret;

	ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);
	if (ret)
		perror("Failed to create endpoint.\n");
	return ret;
}

static char *get_rpmsg_ept_dev_name(const char *rpmsg_char_name,
				    const char *ept_name,
				    char *ept_dev_name)
{
	char sys_rpmsg_ept_name_path[512];
	char svc_name[512];
	char *sys_rpmsg_path = "/sys/class/rpmsg";
	FILE *fp;
	int i;
	int ept_name_len;

	for (i = 0; i < 128; i++) {
		sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
			sys_rpmsg_path, rpmsg_char_name, i);
		printf("checking %s\n", sys_rpmsg_ept_name_path);
		if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
			continue;
		fp = fopen(sys_rpmsg_ept_name_path, "r");
		if (!fp) {
			printf("failed to open %s\n", sys_rpmsg_ept_name_path);
			break;
		}
		fgets(svc_name, sizeof(svc_name), fp);
		fclose(fp);
		printf("svc_name: %s.\n", svc_name);
		
		ept_name_len = strlen(ept_name);
		
		if (ept_name_len > sizeof(svc_name))
			ept_name_len = sizeof(svc_name);
		if (!strncmp(svc_name, ept_name, ept_name_len)) {
			sprintf(ept_dev_name, "rpmsg%d", i);
			return ept_dev_name;
		}
	}

	printf("Not able to RPMsg endpoint file for %s:%s.\n", rpmsg_char_name, ept_name);
	return NULL;
}

static int bind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
	char fpath[256];
	char *rpmsg_chdrv = "rpmsg_chrdev";
	int fd;
	int ret;


	/* rpmsg dev overrides path */
	sprintf(fpath, "%s/devices/%s/driver_override",
		RPMSG_BUS_SYS, rpmsg_dev_name);
	fd = open(fpath, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);
	if (ret < 0) {
		fprintf(stderr, "Failed to write %s to %s, %s\n",
			rpmsg_chdrv, fpath, strerror(errno));
		return -EINVAL;
	}
	close(fd);

	/* bind the rpmsg device to rpmsg char driver */
	sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
	fd = open(fpath, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);
	if (ret < 0) {
		fprintf(stderr, "Failed to write %s to %s, %s\n", rpmsg_dev_name, fpath, strerror(errno));
		return -EINVAL;
	}
	close(fd);
	return 0;
}

static int get_rpmsg_chrdev_fd(const char *rpmsg_dev_name, char *rpmsg_ctrl_name)
{
	char dpath[256];
	char fpath[256];
	char *rpmsg_ctrl_prefix = "rpmsg_ctrl";
	DIR *dir;
	struct dirent *ent;
	int _fd;

	sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name); // sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name);
	dir = opendir(dpath);
	if (dir == NULL) {
		fprintf(stderr, "Failed to open dir %s\n", dpath);
		return -EINVAL;
	}
	while ((ent = readdir(dir)) != NULL) {
		if (!strncmp(ent->d_name, rpmsg_ctrl_prefix,
			    strlen(rpmsg_ctrl_prefix))) {
			printf("Opening file %s.\n", ent->d_name);
			sprintf(fpath, "/dev/%s", ent->d_name);
			_fd = open(fpath, O_RDWR | O_NONBLOCK);
			if (_fd < 0) {
				fprintf(stderr, "Failed to open rpmsg char dev %s,%s\n", fpath, strerror(errno));
				return _fd;
			}
			sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
			return _fd;
		}
	}

	fprintf(stderr, "No rpmsg char dev file is found\n");
	return -EINVAL;
}

int         size, bytes_rcvd, bytes_sent;
int         opt;
char        *rpmsg_dev="virtio0.rpmsg-openamp-demo-channel.-1.0";
int         ntimes = 1;
char        fpath[256];
char        rpmsg_char_name[16];
struct      rpmsg_endpoint_info eptinfo;
char        ept_dev_name[16];
char        ept_dev_path[32];
uint64_t    packet_counter;

int candrv_init(void)
{
	int ret, i, j;
  
  err_cnt = 0;

	//printf("\r\n Echo test start \r\n");
  //printf("sizeof(THE_MESSAGE): %d \r\n", sizeof(THE_MESSAGE));

  ret = 0;

 	/* Try to create endpoint from rpmsg char driver */
  //printf("\r\n Open rpmsg dev %s! \r\n", rpmsg_dev);
  sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);
  
  if (access(fpath, F_OK)) {
	  fprintf(stderr, "Not able to access rpmsg device %s, %s\n", fpath, strerror(errno));
	  ret = -1;
  } 
  else {
    // ret = bind_rpmsg_chrdev(rpmsg_dev);
    // if (ret >= 0) {
      charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);
      if (charfd < 0) {
	      ret = 1;
      }
      else {
	      strcpy( eptinfo.name, "rpmsg-openamp-demo-channel" );
	      eptinfo.src     = 1;	// eptinfo.src = 0;
	      eptinfo.dst     = 0;	// eptinfo.dst = 0xFFFFFFFF;
	      ret             = rpmsg_create_ept( charfd, &eptinfo );
	    }
    // }
  }
	  
	
	if (ret) {

	  /* Load rpmsg_char driver */
	  //printf("\r\nMaster>probe rpmsg_char\r\n");
	  ret = system("modprobe rpmsg_char");
	  if (ret < 0) {
		  perror("Failed to load rpmsg_char driver.\n");
		  return -EINVAL;
	  }

	  //printf("\r\n Open rpmsg dev %s! \r\n", rpmsg_dev);
	  sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);
	  if (access(fpath, F_OK)) {
		  fprintf(stderr, "Not able to access rpmsg device %s, %s\n",
			  fpath, strerror(errno));
		  return -EINVAL;
	  }
	  
	  ret = bind_rpmsg_chrdev(rpmsg_dev);
	  if (ret < 0)
		  return ret;
	  charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);
	  if (charfd < 0)
		  return charfd;
    
	  /* Create endpoint from rpmsg char driver */
	  strcpy(eptinfo.name, "rpmsg-openamp-demo-channel");
	  eptinfo.src     = 1;	// eptinfo.src = 0;
	  eptinfo.dst     = 0;	// eptinfo.dst = 0xFFFFFFFF;

	  ret = rpmsg_create_ept(charfd, &eptinfo);
	  if (ret) {
		  printf("failed to create RPMsg endpoint.\n");
		  return -EINVAL;
	  }
	}
	
	if ( !get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name, ept_dev_name) )
		return -EINVAL;
		
	sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
	fd = open(ept_dev_path, O_RDWR | O_NONBLOCK);
	
	if (fd < 0) {
		perror("Failed to open rpmsg device.");
		close(charfd);
		return -1;
	}

	r_payload = (struct _payload *)malloc( sizeof(_payload) );
	i_payload = (struct _payload *)malloc( sizeof(_payload) );

	if (i_payload == 0 || r_payload == 0) {
	  close(charfd);
	  close(fd);
		printf("ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}

  // printf("RPMSG_MESSAGE_MAX_SIZE = %d\n\r", RPMSG_MESSAGE_MAX_SIZE);

  // send handshake after connecting to M4
  i_payload->head.packet_id    = 0xAA;
  write( fd, i_payload, sizeof(i_payload->head.packet_id) );

  return fd;

}

/*
    uint16_t    m_data_len;         // module data len
    uint32_t    m_func;
    int         m_status;           // additional address
    int         m_idx;              // module index 0..7
    uint16_t    m_addr;             // additional address
*/

/*
+  _K15_DI16_ModuleReadAll         = DevTypeK15_DI16+1,
+  _K15_DI16_ModuleExchange,
+  _K15_DI16_ClearCounter,
+  _K15_DI16_ReceiveCounter,
+  _K15_DI16_ReadData,
+  _K15_DI16_WriteData,
  
+  _K15_DO16_ModuleReadAll         = DevTypeK15_DO16+1,
+  _K15_DO16_ModuleExchange,
+  _K15_DO16_ChSet,
+  _K15_DO16_ReadData,
+  _K15_DO16_WriteData,

+  _K15_AI8_ModuleReadAll          = DevTypeK15_AI8+1,
+  _K15_AI8_ModuleExchange,
+  _K15_AI8_ADC_Enable,
+  _K15_AI8_ADC_Disable,

+  _K15_AO2_ModuleReadAll          = DevTypeK15_AO2+1,
+  _K15_AO2_ModuleExchange,
+  _K15_AO2_DAC1_control,
+  _K15_AO2_DAC2_control,
+  _K15_AO2_WriteData,
  
+  _K15_AI6_ModuleReadAll          = DevTypeK15_AI6+1,
+  _K15_AI6_ModuleExchange,
+  _K15_AI6_ADC_Enable,
+  _K15_AI6_ADC_Disable,
  
+  _K15_DO8pwr_ModuleReadAll      = DevTypeK15_DO8pwr+1,
+  _K15_DO8pwr_ModuleExchange,
+  _K15_DO8pwr_ChSet,
+  _K15_DO8pwr_ReadData,
+  _K15_DO8pwr_WriteData,

+  _K15_NAM_ModuleReadAll          = DevTypeK15_NAMUR+1,
+  _K15_NAM_ModuleExchange,
+  _K15_NAM_WriteData,
  
+  _K19_AI6p_ModuleReadAll         = DevTypeK19_AI6p+1,
+  _K19_AI6p_ModuleExchange,
+  _K19_AI6p_CorrSave,
+  _K19_AI6p_CorrClear,
+  _K19_AI6p_CorrSetDot,
  
+  _K19_AI6a_ModuleReadAll         = DevTypeK19_AI6a+1,
+  _K19_AI6a_ModuleExchange,
+  _K19_AI6a_CorrSave,
+  _K19_AI6a_CorrClear,
+  _K19_AI6a_CorrSetDot,
  
+  _K19_DI16_ModuleReadAll         = DevTypeK19_DI16+1,
+  _K19_DI16_ModuleExchange,
+  _K19_DI16_ClearCounter,
+  _K19_DI16_ReceiveCounter,
+  _K19_DI16_ReadData,
+  _K19_DI16_WriteData,
  
+  _K19_DO16_ModuleReadAll         = DevTypeK19_DO16+1,
+  _K19_DO16_ModuleExchange,
+  _K19_DO16_ReadData,
+  _K19_DO16_WriteData,
  
+  _K19_AO4_ModuleReadAll          = DevTypeK19_AO4+1,
+  _K19_AO4_ModuleExchange,
+  _K19_AO4_ReadData,
+  _K19_AO4_WriteData,
+  _K19_AO4_CalibrationHandler,

+  _K15_DO8_ModuleReadAll          = DevTypeK15_DO8+1,
+  _K15_DO8_ModuleExchange,
+  _K15_DO8_ChSet,
+  _K15_DO8_ReadData,
+  _K15_DO8_WriteData,

+  _K19_DIDO8_ModuleReadAll        = DevTypeK19_DIDO8+1,
+  _K19_DIDO8_ModuleExchange,
+  _K19_DIDO8_ClearCounter,
+  _K19_DIDO8_ReceiveCounter,
+  _K19_DIDO8_ReadData,
+  _K19_DIDO8_WriteData,
  
+  _K15_FDI8_ModuleReadAll         = DevTypeK15_FDI8+1,
+  _K15_FDI8_ModuleExchange,
+  _K15_FDI8_ClearCounter,
+  _K15_FDI8_SetDiFilter,
+  _K15_FDI8_SetFiFilter,
+  _K15_FDI8_ReadData,
+  _K15_FDI8_WriteData,
  
+  _K19_FIO_ModuleReadAll          = DevTypeK19_FIO+1,
+  _K19_FIO_ModuleExchange,
+  _K19_FIO_ClearCounter,
+  _K19_FIO_SetFilter,
+  _K19_FIO_ReadData,
+  _K19_FIO_WriteData,

+  _K19_CRM_ModuleReadAll          = DevTypeK19_CRM+1,
+  _K19_CRM_ModuleExchange,
+  _K19_CRM_SetFilter,
+  _K19_CRM_MeasureStart,
+  _K19_CRM_MeasureStop,

+  _K19_SCM_ModuleReadAll          = DevTypeK19_SCM+1,
+  _K19_SCM_ModuleExchange,
+  _K19_SCM_ReadPacket,
+  _K19_SCM_WritePacket,
+  _K19_SCM_WriteSettings,
*/

int candrv_cmd( uint32_t m_func, int m_idx, void *data )  { // return status /*, uint16_t m_addr */

  int cntr = 0;

  memset( i_payload, 0x55, sizeof(_payload) );
  memset( r_payload, 0, sizeof(_payload) );

  i_payload->head.packet_id         = 0x11223344;
  i_payload->head.packet_counter    = 0xBBCCDDEEAAFF9988;
  i_payload->head.m_func            = m_func;
  i_payload->head.m_idx             = m_idx;
  i_payload->head.m_status          = UL_IN_PROCESS;
  i_payload->head.m_addr            = 0; /* m_addr; */
  
  switch( m_func ) {

    // K15 DI16
    
    case _K15_DI16_ModuleReadAll:
    case _K15_DI16_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_DI16_str);
    break;
    
    case _K15_DI16_ClearCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_DI16_ReceiveCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_DI16_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_DI16_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
    
    // K15 DO16
    
    case _K15_DO16_ChSet:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    case _K15_DO16_ModuleReadAll:
    case _K15_DO16_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_DO16_str);
    break;

    case _K15_DO16_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_DO16_WriteData:
      i_payload->head.m_data_len    = 0;
    break;

    // K15 AI8
    
    case _K15_AI8_ModuleReadAll:
    case _K15_AI8_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_AI8_str);
    break;
    
    case _K15_AI8_ADC_Enable:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_AI8_ADC_Disable:
      i_payload->head.m_data_len    = 0;
    break;
    
    // K15 AO2
    
    case _K15_AO2_ModuleReadAll:
    case _K15_AO2_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_AO2_str);
    break;
    
    case _K15_AO2_DAC1_control:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_AO2_DAC2_control:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_AO2_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
    
    // K15 AI6
    
    case _K15_AI6_ModuleReadAll:
    case _K15_AI6_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_AI6_str);
    break;
    
    case _K15_AI6_ADC_Enable:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_AI6_ADC_Disable:
      i_payload->head.m_data_len    = 0;
    break;

    // K15 DO8 PWR

    case _K15_DO8pwr_ModuleReadAll:
    case _K15_DO8pwr_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_DO8pwr_str);
    break;

    case _K15_DO8pwr_ChSet:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;

    case _K15_DO8pwr_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_DO8pwr_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
    
    // K15 NAMUR
    
    case _K15_NAM_ModuleReadAll:
    case _K15_NAM_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_NAM_str);
    break;
    
    case _K15_NAM_WriteData:
      i_payload->head.m_data_len    = 0;
    break;

    // K19 AI6p
    
    case _K19_AI6p_ModuleReadAll:
    case _K19_AI6p_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_AI6p_str);
    break;

    case _K19_AI6p_CorrSave:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_AI6p_CorrClear:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_AI6p_CorrSetDot:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
      
    // K19 AI6a
    
    case _K19_AI6a_ModuleReadAll:
    case _K19_AI6a_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_AI6a_str);
    break;

    case _K19_AI6a_CorrSave:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_AI6a_CorrClear:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_AI6a_CorrSetDot:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    // K19 DI16
    
    case _K19_DI16_ModuleReadAll:
    case _K19_DI16_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_DI16_str);
    break;
    
    case _K19_DI16_ClearCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_DI16_ReceiveCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_DI16_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_DI16_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
    
    // K19 DO16
    
    case _K19_DO16_ModuleReadAll:
    case _K19_DO16_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_DO16_str);
    break;
    
    case  _K19_DO16_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_DO16_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
    
    // K19 AO4
    
    case _K19_AO4_ModuleReadAll:
    case _K19_AO4_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_AO4_str);
    break;

    case _K19_AO4_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_AO4_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_AO4_CalibrationHandler:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;

    // K15 DO8
    
    case _K15_DO8_ModuleReadAll:
    case _K15_DO8_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_DO8_str);
    break;

    case _K15_DO8_ChSet:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;

    case _K15_DO8_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_DO8_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
  
    // K19 DIDO8

    case _K19_DIDO8_ModuleReadAll:
    case _K19_DIDO8_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_DIDO8_str);
    break;
    
    case _K19_DIDO8_ClearCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_DIDO8_ReceiveCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_DIDO8_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_DIDO8_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
  
    // K19 FDI
  
    case _K15_FDI8_ModuleReadAll:
    case _K15_FDI8_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K15_FDI8_str);
    break;
    
    case _K15_FDI8_ClearCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_FDI8_SetDiFilter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_FDI8_SetFiFilter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K15_FDI8_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K15_FDI8_WriteData:
      i_payload->head.m_data_len    = 0;
    break;
  
    // K19 FIO
  
    case _K19_FIO_ModuleReadAll:
    case _K19_FIO_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_FIO_str);
    break;
    
    case _K19_FIO_ClearCounter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_FIO_SetFilter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_FIO_ReadData:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_FIO_WriteData:
      i_payload->head.m_data_len    = 0;
    break;

    // K19 CRM

    case _K19_CRM_ModuleReadAll:
    case _K19_CRM_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_CRM_str);
    break;
    
    case _K19_CRM_SetFilter:
      i_payload->head.m_data_len    = (uint32_t)sizeof(KXX_str);
    break;
    
    case _K19_CRM_MeasureStart:
      i_payload->head.m_data_len    = 0;
    break;
    
    case _K19_CRM_MeasureStop:
      i_payload->head.m_data_len    = 0;
    break;

    // K19 SCM

    case _K19_SCM_ModuleReadAll:
    case _K19_SCM_ModuleExchange:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_SCM_str);
    break;
    
    case _K19_SCM_ReadPacket:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_SCMex_str);
    break;
    
    case _K19_SCM_WritePacket:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_SCMex_str);
    break;
    
    case _K19_SCM_WriteSettings:
      i_payload->head.m_data_len    = (uint32_t)sizeof(K19_SCMset_str);
    break;
    
    
    default:
      i_payload->head.m_data_len    = 0;
    break;
  }

  // printf("i_payload->head.m_data_len (1) = %d\n\r", i_payload->head.m_data_len);

  if(i_payload->head.m_data_len)
    memcpy(&i_payload->data, data, i_payload->head.m_data_len);
  
  // printf("send data len = %d\n\r", i_payload->head.m_data_len);
  
  // printf("i_payload->head.m_data_len (2) = %d\n\r", i_payload->head.m_data_len);
  
  write( fd, i_payload, RPMSG_MESSAGE_MAX_SIZE );

  // printf("i_payload->head.m_data_len (3) = %d\n\r", i_payload->head.m_data_len);
  
  usleep(500);
  
  bytes_rcvd = 0;
  
	while ( bytes_rcvd <= 0 ) {
		bytes_rcvd = read( fd, r_payload, RPMSG_MESSAGE_MAX_SIZE );
		if(bytes_rcvd > 0) {
 		  // printf("send data len = %d, received data len = %d\n\r", i_payload->head.m_data_len, r_payload->head.m_data_len);
		  if(i_payload->head.m_data_len == r_payload->head.m_data_len && r_payload->head.m_data_len) {
	  	  memcpy( data, &r_payload->data, r_payload->head.m_data_len);
	  	  return 1;
	  	}
	  	if(i_payload->head.m_data_len != r_payload->head.m_data_len) { bytes_rcvd = 0; continue; }
		}
		cntr++;
		if(cntr > 5000) return -1;
		usleep(1000);
	}


}


void candrv_deinit( void ) {
	free(i_payload);
	free(r_payload);

	close(fd);
	if (charfd >= 0)
		close(charfd);
}

