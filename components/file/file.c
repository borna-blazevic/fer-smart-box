/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS nOF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define CHAR_SIZE 1
#define TAG_LENGTH 12
static const char *TAG = "example";

uint8_t* writeToFile(uint8_t* buffer,long int offset,int seekMode){
    ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/extflash/RFIDtags.txt", "w+");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return 0;
    }
    fseek(f,offset,seekMode);
    fwrite(buffer,CHAR_SIZE,TAG_LENGTH,f);
    fclose(f);
    ESP_LOGI(TAG, "File written");
    return TAG_LENGTH;
}

int checkFile(uint8_t* buffer){
  int j=0;
  uint8_t input[TAG_LENGTH];
  bool found = true;
  FILE *f = fopen("/extflash/RFIDtags.txt", "r");
   while (fread(input, CHAR_SIZE, TAG_LENGTH, f) == TAG_LENGTH) {
    for(int i=0;i<TAG_LENGTH;i++)
    {
      if(input[i] != buffer[i]){
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

int deleteFromFile(uint8_t* buffer){
  uint8_t reset[TAG_LENGTH],input;
  for(int i=0;i<TAG_LENGTH;i++)
    reset[i] = 45;
  int positionOf = checkFile(buffer) ;
  if(positionOf == -1)
    return 0;
  FILE *f = fopen("/extflash/RFIDtags.txt", "w+");
  writeToFile(reset,TAG_LENGTH*positionOf,SEEK_SET);
  fclose(f);
  FILE *fileTemp = fopen("/extflash/RFIDtagsTemp.txt", "w+");
  f = fopen("/extflash/RFIDtags.txt", "r+");
  fseek(f,0,SEEK_SET);
  fseek(fileTemp,0,SEEK_SET);
   while (fread(&input, CHAR_SIZE, 1, f) == 1) {
      if(input != 45)
        fwrite(&input,CHAR_SIZE,1,f);
  }
  remove("/extflash/RFIDtags.txt");
  fclose(fileTemp);
  rename("/RFIDtagsTemp.txt","/RFIDtags.txt");
  return 1;
}


void filesystem_task(void *arg)
{
    void (*handler)(void) = (void(*)(void))arg;
	//printf("RADIRADIRADIRADIRADI");
   // ESP_LOGI(TAG, "Initializing SPIFFS");
    
    
    /*
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    printf("%p",&conf);
    while(1);
       // handler();
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    while(1);
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
    */
    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen("/spiffs/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat("/spiffs/foo.txt", &st) == 0) {
        // Delete it if it exists
        unlink("/spiffs/foo.txt");
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/spiffs/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SPIFFS
  //  esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}
void filesystem(void (*handler)(void))
{
    xTaskCreate(filesystem_task, "filesystem_task", 1024, handler, 10, NULL);
}

