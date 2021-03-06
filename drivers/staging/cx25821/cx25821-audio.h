

#ifndef __CX25821_AUDIO_H__
#define __CX25821_AUDIO_H__

#define USE_RISC_NOOP               1
#define LINES_PER_BUFFER            15
#define AUDIO_LINE_SIZE             128


#define NUMBER_OF_PROGRAMS  8




#ifndef USE_RISC_NOOP
#define MAX_BUFFER_PROGRAM_SIZE     \
    (2*LINES_PER_BUFFER*RISC_WRITE_INSTRUCTION_SIZE + RISC_WRITECR_INSTRUCTION_SIZE*4)
#endif


#ifdef USE_RISC_NOOP
#define MAX_BUFFER_PROGRAM_SIZE     \
    (2*LINES_PER_BUFFER*RISC_WRITE_INSTRUCTION_SIZE + RISC_NOOP_INSTRUCTION_SIZE*4)
#endif


#define RISC_WRITE_INSTRUCTION_SIZE 12
#define RISC_JUMP_INSTRUCTION_SIZE  12
#define RISC_SKIP_INSTRUCTION_SIZE  4
#define RISC_SYNC_INSTRUCTION_SIZE  4
#define RISC_WRITECR_INSTRUCTION_SIZE  16
#define RISC_NOOP_INSTRUCTION_SIZE 4

#define MAX_AUDIO_DMA_BUFFER_SIZE (MAX_BUFFER_PROGRAM_SIZE * NUMBER_OF_PROGRAMS + RISC_SYNC_INSTRUCTION_SIZE)

#endif
