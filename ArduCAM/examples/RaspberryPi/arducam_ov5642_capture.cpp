/*-----------------------------------------

//Update History:
//2016/06/13 	V1.1	by Lee	add support for burst mode

--------------------------------------*/
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include "arducam_arch_raspberrypi.h"
#define OV5642_CHIPID_HIGH 0x300a
#define OV5642_CHIPID_LOW 0x300b
#define BUF_SIZE (384*1024)
#define BUF_LEN 4096
#define CAM1_CS 5
uint8_t buffer[BUF_SIZE] = {0xFF};

ArduCAM myCAM(OV5642,CAM1_CS);
void setup()
{
    uint8_t vid,pid;
    uint8_t temp;
    // Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);
    if(temp != 0x55) {
        printf("SPI interface error!\n");
        exit(EXIT_FAILURE);
    }  
    // Change MCU mode
    myCAM.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42))
    printf("Can't find OV5642 module!");
     else
     printf("OV5642 detected.\n");
}
int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Usage: %s [-s <resolution>] | [-c <filename]", argv[0]);
        printf(" -s <resolution> Set resolution, valid resolutions are:\n");
        printf("                   320x240\n");
        printf("                   640x480\n");
        printf("                   1280x960\n");
        printf("                   1600x1200\n");
        printf("                   2048x1536\n");
        printf("                   2592x1944\n");
        printf(" -c <filename>   Capture image\n");
        exit(EXIT_SUCCESS);
    }
  	if (strcmp(argv[1], "-c") == 0 && argc == 4) 
  	{
      setup(); 
      myCAM.set_format(JPEG);
      myCAM.InitCAM();
      // Change to JPEG capture mode and initialize the OV2640 module   
      if (strcmp(argv[3], "320x240")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_320x240);
      else if (strcmp(argv[3], "320x240")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_640x480);
      else if (strcmp(argv[3], "1280x960")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_1280x960);
      else if (strcmp(argv[3], "1600x1200")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);
      else if (strcmp(argv[3], "2048x1536")  == 0) myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);
      else if (strcmp(argv[3], "2592x1944") == 0) myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);
      else {
      printf("Unknown resolution %s\n", argv[3]);
      exit(EXIT_FAILURE);
      }
      sleep(1); // Let auto exposure do it's thing after changing image settings
      printf("Changed resolution1 to %s\n", argv[3]); 
      myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH   	  
      // Flush the FIFO
      myCAM.flush_fifo();    
      // Clear the capture done flag
      myCAM.clear_fifo_flag();
      // Start capture
      printf("Start capture\n");  
      myCAM.start_capture();
      while (!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) ;
      printf("CAM Capture Done\n");
              
       // Open the new file
      FILE *fp1 = fopen(argv[2], "w+");   
      if (!fp1) {
          printf("Error: could not open %s\n", argv[2]);
          exit(EXIT_FAILURE);
      }
       
      printf("Reading FIFO and saving IMG\n");    
      size_t len = myCAM.read_fifo_length();
      if (len >= MAX_FIFO_SIZE){
		   printf("Over size.");
		    exit(EXIT_FAILURE);
		  }else if (len == 0 ){
		    printf("Size is 0.");
		    exit(EXIT_FAILURE);
		  } 
	  myCAM.CS_LOW();  //Set CS low       
      myCAM.set_fifo_burst();
      myCAM.transfers(buffer,1);//dummy read  
      int32_t i=0;
      while(len>BUF_LEN)
      {	 
      	myCAM.transfers(&buffer[i],BUF_LEN);
      	len -= BUF_LEN;
      	i += BUF_LEN;
      }
      myCAM.transfers(&buffer[i],len); 
      myCAM.CS_HIGH();//Set CS HIGH
      fwrite(buffer, len+i, 1, fp1);
       //Close the file
      delay(100);
      fclose(fp1);  
      // Clear the capture done flag
      myCAM.clear_fifo_flag();
	  printf("IMG save OK !\n");

  } else {
      printf("Error: unknown or missing argument.\n");
      exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
