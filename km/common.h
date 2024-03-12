
#ifndef COMMON_H__
#define COMMON_H__

#define BUFFER_SIZE 128
#define DEBUG_READ_FROM_US 1
#define LOG_PROC_FILE_PREFIX "AVM: Proc-file"

static unsigned char *internal_storage;
// the current text's length that is stored internally
static size_t current_storage_pos;

#endif