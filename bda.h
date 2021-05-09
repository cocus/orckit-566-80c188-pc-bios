// Copyright Jamie Iles, 2017
//
// This file is part of s80x86.
//
// s80x86 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// s80x86 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with s80x86.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "helpers.h"
#include "io.h"
#include <stdint.h>

struct __attribute__((packed)) bios_data_area {
    uint16_t com1_port_address;             ///< COM1 port address
    uint16_t com2_port_address;             ///< COM2 port address
    uint16_t com3_port_address;             ///< COM3 port address
    uint16_t com4_port_address;             ///< COM4 port address
    uint16_t lpt1_port_address;                ///< LPT1 port address
    uint16_t lpt2_port_address;                ///< LPT2 port address
    uint16_t lpt3_port_address;                ///< LPT3 port address
    uint16_t lpt4_port_address;                ///< LPT4 port address (except PS/2)
    uint16_t equipment_list_flags;          ///< Equipment list flags (see INT 11)
    uint8_t pcjr_ir_kb_link_err_cnt;        ///< PCjr: infrared keyboard link error count
    uint16_t mem_kbytes;                    ///< Memory size in Kbytes  (see INT 12)
    uint8_t reserved1;                      ///< Reserved
    uint8_t ps2_bios_control_flags;         ///< PS/2 BIOS control flags
    uint8_t keyboard_flag_byte_0;           ///< Keyboard flag byte 0 (see KB FLAGS)
    uint8_t keyboard_flag_byte_1;           ///< Keyboard flag byte 1 (see KB FLAGS)
    uint8_t keypad_entry;                   ///< Storage for alternate keypad entry
    uint16_t kbd_buffer_head_offset;        ///< Offset from 40:00 to keyboard buffer head
    uint16_t kbd_buffer_tail_offset;        ///< Offset from 40:00 to keyboard buffer tail
    uint16_t kbd_buffer[16];                ///< Keyboard buffer (circular queue buffer)
    uint8_t drive_recalibration_status;     ///< Drive recalibration status
    uint8_t diskette_motor_status;          ///< Diskette motor status
    uint8_t motor_shutoff_count;            ///< Motor shutoff counter (decremented by INT 8)
    uint8_t diskette_status;                ///< Status of last diskette operation (see INT 13,1)
    uint8_t nec_diskette_motor_status[7];   ///< NEC diskette controller status (see FDC)
    uint8_t video_mode;                     ///< Current video mode  (see VIDEO MODE)
    uint16_t num_screen_cols;               ///< Number of screen columns
    uint16_t video_regen_buffer_bytes;      ///< Size of current video regen buffer in bytes
    uint16_t video_regen_offset;            ///< Offset of current video page in video regen buffer
    uint16_t cursor_offsets[8];             ///< Cursor position of pages 1-8, high order byte=row
                                            ///  low order byte=column; changing this data isn't
                                            ///  reflected immediately on the display
    uint8_t cursor_end;                     ///< Cursor ending (bottom) scan line (don't modify)
    uint8_t cursor_start;                   ///< Cursor starting (top) scan line (don't modify)
    uint8_t active_page;                    ///< Active display page number
    uint16_t crt_controller_base;           ///< Base port address for active 6845 CRT controller
                                            ///  3B4h = mono, 3D4h = color
    uint8_t crt_mode_control;               ///< 6845 CRT mode control register value (port 3x8h)
                                            ///  EGA/VGA values emulate those of the MDA/CGA
    uint8_t cga_pallette_mask;              ///< CGA current color palette mask setting (port 3d9h)
                                            ///  EGA and VGA values emulate the CGA
    uint8_t cassette_take_ctrl[5];          ///< Cassette tape control (before AT)
    uint32_t timer_counter;                 ///< Daily timer counter, equal to zero at midnight;
                                            ///  incremented by INT 8; read/set by INT 1A
    uint8_t clock_rollover;                 ///< Clock rollover flag, set when 40:6C exceeds 24hrs
    uint8_t break_hit;                      ///< BIOS break flag, bit 7 is set if Ctrl-Break was
                                            ///  *ever* hit; set by INT 9
    uint16_t soft_reset_flag;               ///< Soft reset flag via Ctl-Alt-Del or JMP FFFF:0
                                            ///  1234h  Bypass memory tests & CRT initialization
                                            ///  4321h  Preserve memory
                                            ///  5678h  System suspend
                                            ///  9ABCh  Manufacturer test
                                            ///  ABCDh  Convertible POST loop
                                            ///  ????h  many other values are used during POST
    uint8_t hard_disk_status;               ///< Status of last hard disk operation (see INT 13,1)
    uint8_t num_hard_disks;                 ///< Number of hard disks attached
    uint8_t fixed_disk_control;             ///< XT fixed disk drive control byte
    uint8_t fixed_disk_controller_port;     ///< Port offset to current fixed disk adapter
    uint32_t parallel_timeout;              ///< Time-Out value for LPT1,LPT2,LPT3(,LPT4 except PS/2)
    uint32_t serial_timeout;                ///< Time-Out value for COM1,COM2,COM3,COM4
    uint16_t keyboard_buffer_start;         ///< Keyboard buffer start offset (seg=40h,BIOS 10-27-82)
    uint16_t keyboard_buffer_end;           ///< Keyboard buffer end offset (seg=40h,BIOS 10-27-82)
    uint8_t last_screen_row;                ///< Rows on the screen (less 1, EGA+)
    uint8_t char_point_height;              ///< Point height of character matrix (EGA+, overlaps pcjr_repeat_delay)
    uint8_t pcjr_repeat_delay;              ///< PCjr: initial delay before repeat key action begins
    uint8_t video_mode_options;             ///< PCjr: current Fn function key number or,
                                            ///  Video mode options (EGA+)
    uint8_t ega_feature_bits;               ///< PCjr: third keyboard status byte or,
                                            ///  EGA feature bit switches, emulated on VGA
    uint8_t video_display_data_area;        ///< Video display data area (MCGA and VGA)
    uint8_t dcc_index;                      ///< Display Combination Code (DCC) table index (EGA+)
    uint8_t last_diskette_data_rate;        ///< Last diskette data rate selected
    uint8_t hard_disk_status_from_cntrllr;  ///< Hard disk status returned by controller
    uint8_t hard_disk_error_from_cntrllr;   ///< Hard disk error returned by controller
    uint8_t hard_disk_int_control_flag;     ///< Hard disk interrupt control flag(bit 7=working int)
    uint8_t hdd_fdd_combo_card;             ///< Combination hard/floppy disk card when bit 0 set
    uint8_t drive_0_media_state;            ///< Drive 0 media state
    uint8_t drive_1_media_state;            ///< Drive 1 media state
    uint8_t drive_2_media_state;            ///< Drive 2 media state
    uint8_t drive_3_media_state;            ///< Drive 3 media state
    uint8_t track_seeked_drive_0;           ///< Track currently seeked to on drive 0
    uint8_t track_seeked_drive_1;           ///< Track currently seeked to on drive 1
    uint8_t kb_mode_type;                   ///< Keyboard mode/type
    uint8_t kb_led_flags;                   ///< Keyboard LED flags
};

#define bda_write(field, val)                                                \
    ({                                                                       \
        typeof(((struct bios_data_area *)0)->field) _p = (val);              \
        if (__builtin_types_compatible_p(typeof(_p), uint16_t))        \
            writew(0x40, offsetof(struct bios_data_area, field), _p);        \
        else if (__builtin_types_compatible_p(typeof(_p), uint8_t))    \
            writeb(0x40, offsetof(struct bios_data_area, field), _p);        \
        else                                                                 \
            memcpy_seg(0x40, (void *)offsetof(struct bios_data_area, field), \
                       get_cs(), &_p, sizeof(_p));                           \
    })

#define bda_read(field)                                                   \
    ({                                                                    \
        typeof(((struct bios_data_area *)0)->field) _p;                   \
        if (__builtin_types_compatible_p(typeof(_p), uint16_t))     \
            _p = readw(0x40, offsetof(struct bios_data_area, field));     \
        else if (__builtin_types_compatible_p(typeof(_p), uint8_t)) \
            _p = readb(0x40, offsetof(struct bios_data_area, field));     \
        else                                                              \
            memcpy_seg(get_cs(), &_p, 0x40,                               \
                       (void *)offsetof(struct bios_data_area, field),    \
                       sizeof(_p));                                       \
        _p;                                                               \
    })


