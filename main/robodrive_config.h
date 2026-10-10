#pragma once

// Board revision comes from the build: idf.py -B build_revA -DROBODRIVE_BOARD_REV=A build
#if defined(ROBODRIVE_BOARD_REV_A)
    #define ROBODRIVE_BOARD_REV 'A'
#elif defined(ROBODRIVE_BOARD_REV_B)
    #define ROBODRIVE_BOARD_REV 'B'
#else
    #error "Board revision not set, build with -DROBODRIVE_BOARD_REV=A or B"
#endif
