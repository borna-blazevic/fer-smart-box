#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define CHAR_SIZE 1
#define TAG_LENGTH 12


static const char *TAG = "File";
void filesystem_init(){
    ESP_LOGI(TAG, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    ESP_LOGI(TAG, "SPIFFS initialized");
   
    
   
}

int fileWrite(uint8_t* buffer,long int offset,int seekMode){
    //ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/spiffs/RFIDtags.txt", "a+");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return 0;
    }
    fseek(f,offset,seekMode);
    fwrite(buffer,CHAR_SIZE,TAG_LENGTH+1,f);
    fclose(f);
    //ESP_LOGI(TAG, "File written");
    return TAG_LENGTH;
}

int replaceFile(char * buffer, int length){

  remove("/spiffs/RFIDtags.txt");
  int ret;
  FILE *f = fopen("/spiffs/RFIDtags.txt", "a");
  if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return 0;
    }
  fseek(f, 0, SEEK_SET);
  ret = fwrite(buffer, CHAR_SIZE, length+1, f);
  ESP_LOGE(TAG, "ret %d", ret);
  fclose(f);
  return ret;
}

int fileFind(uint8_t* buffer){
  int j=0;
  uint8_t *data;
  long fsize;
  bool found = true;
  FILE *f = fopen("/spiffs/RFIDtags.txt", "r");
  if(f== NULL)
  {
    ESP_LOGI(TAG, "file does not exist");
    fclose(f);
    return -1;
  }
  fseek(f,0,SEEK_END);
  fsize = ftell(f);
  data =(uint8_t*)malloc(fsize);
  fseek(f,0,SEEK_SET);
  fgets((char*)data, fsize, f); 
  for(int k=0;k<fsize;k+=TAG_LENGTH){

    for(int i=0;i<TAG_LENGTH;i++)
    {
      if(data[i+k] != buffer[i]){
        found = false;  
    }
  }
  if(found){
      fclose(f);
      return j;
  }else{
      found = true;
      j++;
      continue;
  }}
  fclose(f);
  return -1;
}

int fileDelete(uint8_t* buffer){
  uint8_t reset[TAG_LENGTH],input;
  for(int i=0;i<TAG_LENGTH;i++)
    reset[i] = 45;
  int positionOf = fileFind(buffer) ;
  if(positionOf == -1)
    return 0;
  FILE *f = fopen("/spiffs/RFIDtags.txt", "w+");
  fileWrite(reset,TAG_LENGTH*positionOf,SEEK_SET);
  fclose(f);
  FILE *fileTemp = fopen("/spiffs/RFIDtagsTemp.txt", "w+");
  f = fopen("/spiffs/RFIDtags.txt", "r+");
  fseek(f,0,SEEK_SET);
  fseek(fileTemp,0,SEEK_SET);
   while (fread(&input, CHAR_SIZE, 1, f) == 1) {
      if(input != 45)
        fwrite(&input,CHAR_SIZE,1,f);
  }
  remove("/spiffs/RFIDtags.txt");
  fclose(fileTemp);
  rename("/spiffs/RFIDtagsTemp.txt","/spiffs/RFIDtags.txt");
  return 1;
}

void deleteRFIDfile(){
   struct stat st;
    if (stat("/spiffs/RFIDtags.txt", &st) == 0) {
        // Delete it if it exists
        unlink("/spiffs/RFIDtags.txt");
          ESP_LOGI(TAG, "Deleted RFIDtags.txt");
    }
}

void read_file(){
  int j=0;
  uint8_t *data;
  long fsize;
  bool found = true;
  FILE *f = fopen("/spiffs/RFIDtags.txt", "r");
  if(f== NULL)
  {
    ESP_LOGI(TAG, "file does not exist");
    return;
  }
  fseek(f,0,SEEK_END);
  fsize = ftell(f);
  data =(uint8_t*)malloc(fsize);
  fseek(f,0,SEEK_SET);
  fgets((char*)data, fsize, f); 
  ESP_LOGI(TAG, "file : %s", data);
  return;
  
}