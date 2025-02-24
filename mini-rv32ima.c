// Copyright 2022 Charles Lohr, you may use this file or any portions herein under any of the BSD, MIT, or CC0 licenses.

// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
#include <string.h>
// #include <math.h>

// #include "baremetal.h"
#include "linux.h"

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;


static char digits[] = "0123456789abcdef";


#include <stdarg.h>

// void * memcpy(void *dest, const void *src, uint32_t n) {
//     char *d = (char *)dest;
//     const char *s = (const char *)src;
//     for (uint32_t i = 0; i < n; i++) {
//         d[i] = s[i];
//     }
//     return dest;
// }

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    int chars_printed = 0;

    while (*format) {
        if (*format == '%') {
            format++; // Move past the '%'

            if (*format == '0') {
              format++; // Move past the '0'
              if(*format == '8'){
                format++; //Move past the '8'
                if(*format == 'x'){
                  format++; //Move past the 'x'
                  unsigned int val = va_arg(args, unsigned int);
                  for (int i = 7; i >= 0; i--) {
                      unsigned int nibble = (val >> (i * 4)) & 0xF;
                      char hex_digit = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'a');
                      putchar(hex_digit);
                      chars_printed++;
                  }
                } else {
                  //Handle other cases or error
                  putchar('%');
                  putchar(*format);
                  chars_printed+=2;
                }
              } else {
                  //Handle other cases or error
                  putchar('%');
                  putchar('0');
                  putchar(*format);
                  chars_printed+=3;
              }

            } else if (*format == 's') {
                format++;
                char *str = va_arg(args, char *);
                if (str) {
                    while (*str) {
                        putchar(*str);
                        str++;
                        chars_printed++;
                    }
                }
            } else if (*format == 'c') {
                format++;
                int c = va_arg(args, int);
                putchar(c);
                chars_printed++;
            } else if (*format == 'd' || *format == 'i') {
                format++;
                int num = va_arg(args, int);
                char str[12]; // Enough for -2147483648
                int len = 0;
                int is_negative = 0;

                if (num < 0) {
                    is_negative = 1;
                    num = -num;
                }

                do {
                    str[len] = (num % 10) + '0';
					len++;
                    num /= 10;
                } while (num > 0);

                if (is_negative) {
                    putchar('-');
                    chars_printed++;
                }

                for (int i = len - 1; i >= 0; i--) {
					// putchar('x');
                    putchar(str[i]);
                    chars_printed++;
                }
            } else if (*format == '%') {
                putchar('%');
                format++;
                chars_printed++;
            } else {
                // Handle unsupported format specifiers or errors
                putchar('%');
                putchar(*format);
                format++;
                chars_printed+=2;
            }
        } else {
            putchar(*format);
            format++;
            chars_printed++;
        }
    }

    va_end(args);
	// fflush(stdout);

    return chars_printed;
}


int isdigit(int c) {
  return (c >= '0' && c <= '9');
}
int isxdigit(int c) {
  return (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}
int islower(int c) {
  return (c >= 'a' && c <= 'z');
}

int isspace(int c) {
	return c == ' ';
}

uint64_t strtoll(const char* strSource, char** endptr, int base) {
    const char* p = strSource;
    uint64_t result = 0;
    int sign = 1;
    int overflow = 0;
	int errno = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Check for sign
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    // Determine base if it's 0
    if (base == 0) {
        if (*p == '0') {
            p++;
            if (*p == 'x' || *p == 'X') {
                base = 16;
                p++;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base < 2 || base > 36) {
        if (endptr != 0) {
            *endptr = (char*)strSource;
        }
        errno = 1;
        return 0;
    }

    // Convert digits
    while (isxdigit((unsigned char)*p)) {
        int digit;
        if (isdigit((unsigned char)*p)) {
            digit = *p - '0';
        } else if (islower((unsigned char)*p)) {
            digit = *p - 'a' + 10;
        } else {
            digit = *p - 'A' + 10;
        }

        if (digit >= base) {
            break;
        }
    
        if (result > (__INT64_MAX__ / base) || (result == (__INT64_MAX__ / base) && digit > (__INT64_MAX__ % base)))
        {
            overflow = 1;
            if (sign == 1)
            {
                result = __INT64_MAX__;
            } else {
                result = 0;
            }
            break;
        }
        
        result = result * base + digit;
        p++;
    }
    
    if (overflow) {
        errno = 1;
    } else {
        errno = 0;
    }

    if (endptr != 0) {
        *endptr = (char*)p;
    }

    return result * sign;
}

#include "default64mbdtc.h"

// Just default RAM amount is 64MB.
int fail_on_all_faults = 0;

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber );
int lasttimex = 0;
static uint64_t GetTimeMicroseconds() {
	lasttimex = lasttimex+1;
	// printf("time %d\n",lasttimex);
	return lasttimex;
}
static void ResetKeyboardInput();
static void CaptureKeyboardInput();
static uint32_t HandleException( uint32_t ir, uint32_t retval );
static uint32_t HandleControlStore( uint32_t addy, uint32_t val );
static uint32_t HandleControlLoad( uint32_t addy );
static void HandleOtherCSRWrite( uint8_t * image, uint16_t csrno, uint32_t value );
static int32_t HandleOtherCSRRead( uint8_t * image, uint16_t csrno );
static void MiniSleep();
static int IsKBHit();
static int ReadKBByte();

// This is the functionality we want to override in the emulator.
//  think of this as the way the emulator's processor is connected to the outside world.
#define MINIRV32WARN( x... ) printf( x );
#define MINIRV32_DECORATE  static
#define MINI_RV32_RAM_SIZE ram_amt
#define MINIRV32_IMPLEMENTATION
#define MINIRV32_POSTEXEC( pc, ir, retval ) { if( retval > 0 ) { if( fail_on_all_faults ) { printf( "FAULT\n" ); return 3; } else retval = HandleException( ir, retval ); } }
#define MINIRV32_HANDLE_MEM_STORE_CONTROL( addy, val ) if( HandleControlStore( addy, val ) ) return val;
#define MINIRV32_HANDLE_MEM_LOAD_CONTROL( addy, rval ) rval = HandleControlLoad( addy );
#define MINIRV32_OTHERCSR_WRITE( csrno, value ) HandleOtherCSRWrite( image, csrno, value );
#define MINIRV32_OTHERCSR_READ( csrno, value ) value = HandleOtherCSRRead( image, csrno );

#include "mini-rv32ima.h"


// static volatile uint8_t ram_image[0x6000000];
struct MiniRV32IMAState * core;
const char kernel_command_line[] = "console=hvc0 \0                                                             ";

static void DumpState( struct MiniRV32IMAState * core, uint8_t * ram_image );

int main( int argc, char ** argv )
{
	int i;
	long long instct = -1;
	int show_help = 0;
	int time_divisor = 1;
	int fixed_update = 0;
	int do_sleep = 1;
	int single_step = 0;
	int dtb_ptr = 0;
	const char * image_file_name = 0;
	const char * dtb_file_name = 0;
	for( i = 1; i < argc; i++ )
	{
		const char * param = argv[i];
		int param_continue = 0; // Can combine parameters, like -lpt x
		do
		{
			if( param[0] == '-' || param_continue )
			{
				switch( param[1] )
				{
				// case 'm': if( ++i < argc ) ram_amt = SimpleReadNumberInt( argv[i], ram_amt ); break;
				case 'c': if( ++i < argc ) instct = SimpleReadNumberInt( argv[i], -1 ); break;
				// case 'k': if( ++i < argc ) kernel_command_line = argv[i]; break;
				case 'f': image_file_name = (++i<argc)?argv[i]:0; break;
				case 'b': dtb_file_name = (++i<argc)?argv[i]:0; break;
				case 'l': param_continue = 1; fixed_update = 1; break;
				case 'p': param_continue = 1; do_sleep = 0; break;
				case 's': param_continue = 1; single_step = 1; break;
				case 'd': param_continue = 1; fail_on_all_faults = 1; break; 
				case 't': if( ++i < argc ) time_divisor = SimpleReadNumberInt( argv[i], 1 ); break;
				default:
					if( param_continue )
						param_continue = 0;
					else
						show_help = 1;
					break;
				}
			}
			else
			{
				show_help = 1;
				break;
			}
			param++;
		} while( param_continue );
	}
	if( show_help || image_file_name == 0 || time_divisor <= 0 )
	{
		printf(  "./mini-rv32imaf [parameters]\n\t-m [ram amount]\n\t-f [running image]\n\t-k [kernel command line]\n\t-b [dtb file, or 'disable']\n\t-c instruction count\n\t-s single step with full processor state\n\t-t time divion base\n\t-l lock time base to instruction count\n\t-p disable sleep when wfi\n\t-d fail out immediately on all faults\n" );
		// return 1;
	}

	// ram_image = malloc( ram_amt );
	// if( !ram_image )
	// {
		// fail_on_all_faults = 1; 
		printf("Using builtin ramimage\n");
		// fflush(stdout);
		// memcpy(ram_image,bindata,ram_amt);
		// ram_image = bindata;

		// printf(  "Error: could not allocate system image.\n" );
		// return -4;
	// }

restart:
	{

		// Load a default dtb.
		dtb_ptr = ram_amt - sizeof(default64mbdtb) - sizeof( struct MiniRV32IMAState );
		memcpy( ram_image + dtb_ptr, default64mbdtb, sizeof( default64mbdtb ) );

		memcpy( (char*)( ram_image + dtb_ptr + 0xc0 ), kernel_command_line, 54);
		printf("cmdline %x\n",ram_image + dtb_ptr + 0xc0);
		printf("cmdline %s\n",ram_image + dtb_ptr + 0xc0);
		// memcpy( (char*)( ram_image + dtb_ptr + 0xc0 ), kernel_command_line, 54 );
		
	}

	// CaptureKeyboardInput();

	// The core lives at the end of RAM.
	core = (struct MiniRV32IMAState *)(ram_image + ram_amt - sizeof( struct MiniRV32IMAState ));
	core->pc = MINIRV32_RAM_IMAGE_OFFSET;
	core->regs[10] = 0x00; //hart ID
	core->regs[11] = dtb_ptr?(dtb_ptr+MINIRV32_RAM_IMAGE_OFFSET):0; //dtb_pa (Must be valid pointer) (Should be pointer to dtb)
	core->extraflags |= 3; // Machine-mode.

		// Update system ram size in DTB (but if and only if we're using the default DTB)
		// Warning - this will need to be updated if the skeleton DTB is ever modified.
	uint32_t * dtb = (uint32_t*)(ram_image + dtb_ptr);
	if( dtb[0x13c/4] == 0x00c0ff03 )
	{
		uint32_t validram = dtb_ptr;
		dtb[0x13c/4] = (validram>>24) | ((( validram >> 16 ) & 0xff) << 8 ) | (((validram>>8) & 0xff ) << 16 ) | ( ( validram & 0xff) << 24 );
	}

	// Image is loaded.
	uint64_t rt;
	uint64_t lastTime = (fixed_update)?0:(GetTimeMicroseconds()/time_divisor);//
	int instrs_per_flip = 1;// single_step?1:1024;
	for( rt = 0; rt < instct+1 || instct < 0; rt += instrs_per_flip )
	{
		uint64_t * this_ccount = ((uint64_t*)&core->cyclel);
		uint32_t elapsedUs = 0;
		if( fixed_update )
			elapsedUs = *this_ccount / time_divisor - lastTime;
		else
			elapsedUs = GetTimeMicroseconds()/time_divisor - lastTime;// 
		lastTime += elapsedUs;

		if( single_step )
			DumpState( core, ram_image);

		int ret = MiniRV32IMAStep( core, ram_image, 0, elapsedUs, instrs_per_flip ); // Execute upto 1024 cycles before breaking out.
		// printf("ret %d",ret);
		// fflush(stdout);
		switch( ret )
		{
			case 0: break;
			case 1: *this_ccount += instrs_per_flip; break;
			case 3: instct = 0; break;
			case 0x7777: goto restart;	//syscon code for restart
			case 0x5555: printf( "POWEROFF@0x%08x%08x\n", core->cycleh, core->cyclel ); return 0; //syscon code for power-off
			default: printf( "Unknown failure\n" ); break;
		}
	}

	DumpState( core, ram_image);
}


//////////////////////////////////////////////////////////////////////////
// Platform-specific functionality
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Rest of functions functionality
//////////////////////////////////////////////////////////////////////////

static uint32_t HandleException( uint32_t ir, uint32_t code )
{
	// Weird opcode emitted by duktape on exit.
	if( code == 3 )
	{
		// Could handle other opcodes here.
	}
	return code;
}

static uint32_t HandleControlStore( uint32_t addy, uint32_t val )
{
	// printf("here\n");
	if( addy == 0x10000000 ) //UART 8250 / 16550 Data Buffer
	{
		printf( "%c", val );
		// fflush(stdout);

	}
	else if( addy == 0x11004004 ) //CLNT
		core->timermatchh = val;
	else if( addy == 0x11004000 ) //CLNT
		core->timermatchl = val;
	else if( addy == 0x11100000 ) //SYSCON (reboot, poweroff, etc.)
	{
		core->pc = core->pc + 4;
		return val; // NOTE: PC will be PC of Syscon.
	}
	return 0;
}


static uint32_t HandleControlLoad( uint32_t addy )
{
	// Emulating a 8250 / 16550 UART
	// printf("here2\n");

	if( addy == 0x10000005 )
		return 0x60 | 0;//IsKBHit();
	else if( addy == 0x10000000 && 1)//IsKBHit() )
		return getchar();//ReadKBByte();
	else if( addy == 0x1100bffc ) // https://chromitem-soc.readthedocs.io/en/latest/clint.html
		return core->timerh;
	else if( addy == 0x1100bff8 )
		return core->timerl;
	return 0;
}

static void HandleOtherCSRWrite( uint8_t * image, uint16_t csrno, uint32_t value )
{
	if( csrno == 0x136 )
	{
		printf( "%d", value ); 
	}
	if( csrno == 0x137 )
	{
		printf( "%08x", value ); 
	}
	else if( csrno == 0x138 )
	{
		//Print "string"
		uint32_t ptrstart = value - MINIRV32_RAM_IMAGE_OFFSET;
		uint32_t ptrend = ptrstart;
		if( ptrstart >= ram_amt )
			printf( "DEBUG PASSED INVALID PTR (%08x)\n", value );
		while( ptrend < ram_amt )
		{
			if( image[ptrend] == 0 ) break;
			ptrend++;
		}
		if( ptrend != ptrstart ) {
			// printf("FILLIN\n");
			char* ptr = (char*)(image + ptrstart);
			for (int i = 0; i< ptrend - ptrstart; i++) {
				printf("%c",*(ptr+i));
			}
						// fwrite( image + ptrstart, ptrend - ptrstart, 1, stdout );

		}
	}
	else if( csrno == 0x139 )
	{
		putchar( value ); 
	}
}

static int32_t HandleOtherCSRRead( uint8_t * image, uint16_t csrno )
{
	if( csrno == 0x140 )
	{
		// if( !IsKBHit() ) return -1;
		char x = getchar();
		if (x==0) return -1;
		return getchar();//ReadKBByte();
	}
	return 0;
}

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber )
{
	if( !number || !number[0] ) return defaultNumber;
	int radix = 10;
	if( number[0] == '0' )
	{
		char nc = number[1];
		number+=2;
		if( nc == 0 ) return 0;
		else if( nc == 'x' ) radix = 16;
		else if( nc == 'b' ) radix = 2;
		else { number--; radix = 8; }
	}
	char * endptr;
	uint64_t ret = strtoll( number, &endptr, radix );
	if( endptr == number )
	{
		return defaultNumber;
	}
	else
	{
		return ret;
	}
}

static void DumpState( struct MiniRV32IMAState * core, uint8_t * ram_image )
{
	uint32_t pc = core->pc;
	uint32_t pc_offset = pc - MINIRV32_RAM_IMAGE_OFFSET;
	uint32_t ir = 0;

	printf( "PC: %08x ", pc );
	if( pc_offset >= 0 && pc_offset < ram_amt - 3 )
	{
		ir = *((uint32_t*)(&((uint8_t*)ram_image)[pc_offset]));
		printf( "[0x%08x] ", ir ); 
	}
	else
		printf( "[xxxxxxxxxx] " ); 
	uint32_t * regs = core->regs;
	printf( "Z:%08x ra:%08x sp:%08x gp:%08x tp:%08x t0:%08x t1:%08x t2:%08x s0:%08x s1:%08x a0:%08x a1:%08x a2:%08x a3:%08x a4:%08x a5:%08x ",
		regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7],
		regs[8], regs[9], regs[10], regs[11], regs[12], regs[13], regs[14], regs[15] );
	printf( "a6:%08x a7:%08x s2:%08x s3:%08x s4:%08x s5:%08x s6:%08x s7:%08x s8:%08x s9:%08x s10:%08x s11:%08x t3:%08x t4:%08x t5:%08x t6:%08x\n",
		regs[16], regs[17], regs[18], regs[19], regs[20], regs[21], regs[22], regs[23],
		regs[24], regs[25], regs[26], regs[27], regs[28], regs[29], regs[30], regs[31] );
}

