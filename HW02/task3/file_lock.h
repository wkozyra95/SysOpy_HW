#ifndef HW02_FILE_LOCK_H
#define HW02_FILE_LOCK_H

int open_file(const char *path);
void close_file();
void list();
void lock_read(int number);
void lock_write(int number);
void unlock(int number);
void get_sign(int number);
void set_sign(int number, char *character);

#endif //HW02_FILE_LOCK_H
