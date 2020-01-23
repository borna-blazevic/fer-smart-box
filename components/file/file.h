void filesystem_init();
void deleteRFIDfile();
int fileWrite(uint8_t* buffer,long int offset,int seekMode);
int fileFind(uint8_t* buffer);
int fileDelete(uint8_t* buffer);
void read_file();
int replaceFile(char * buffer, int length);