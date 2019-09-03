/*!
*\file mem_main.c
* 
*\brief  Memory module test main function for Command Line Interface
*
*\author Maggie Tong <maggie_tong@jabil.com>
*
* Copyright(c) 2011 Jabil Circuit.
*
* This source code and any compilation or derivative thereof is the sole property of 
* Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
* is the proprietary information of Jabil Circuit and is confidential in nature. 
* It's use and dissemination by any party other than Jabil Circuit is strictly 
* limited by the confidential information provisions of Software License Agreement 
* referenced above.
*
*\par ChangeLog:
* Nov. 25, 2009 0.1.1:   Create this file
* Feb. 11, 2011 : Revise for the new memory test module
*/

/*
 * INCLUDE FILES
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include "jedi_comm.h"
#include "memory.h"
#include "log.h"
#include "version.h"
#include "mem_err.h"
#include "utils.h"

/*
 * LOCAL DEFINES
 */
#define MSG_WIDTH       64
#define NAME_GETINFO    "getinfo"

char *test_items[ITEM_COUNT] = {
    "Address test, walking ones",
    "Address test, own address",
    "Random address test",
    "Moving inversions, ones&zeros",
    "Moving inversions, 8 bit pattern",
    "Moving inversions, random pattern",
    "Block move, 64 moves",
    "Moving inversions, long pattern",
    "Random number sequence",
    "Modulo 20, ones&zeros",
};

typedef struct _errorcodes
{
    unsigned int module_code;
    int cli_code;
}errorcodes;

errorcodes code_map[] = { 
    /*map the module error code to cli error code */
//    {0x00000000, 0},    /* program was complete successful */
    {0xA0000001, 1},    /* Current user have no enough privilege */
    {0xA0000002, 502},  
    {0xA0000003, 22},   /* Parameter were not correct */
    {0xA0000004, 504},
    {0xA0000005, 505},
    {0xA0000006, 6},    /* Driver not exist */


    {0xA0000007, 507},
    {0xA0000008, 508},    
    {0xA0000009, 509},    
    {0xA000000a, 510},    
    {0xA000000b, 511},    
    {0xA000000c, 512},    
    {0xA000000d, 513},    
    {0xA000000e, 514},    
    {0xA000000f, 515},    

    {0xA1000010, 520},   
    {0xA1000011, 521},   
    {0xA1000012, 522},   
    {0xA1000013, 523},   
    {0xA1000014, 524},   
    {0xA1000015, 525},   
    {0xA1000016, 526},   
    {0xA1000017, 527},   
    {0xA1000018, 528},   
    {0xA1000019, 529},   
    {0xA1000020, 530},   
    {0xA1000021, 531},   
    {0xA1000022, 532},   
    {0xA1000023, 533},   
    {0xA1000024, 534},   
    {0xA1000025, 535},   
    {0xA1000026, 536},   
    {0xA100000a, 537},   



    {0xAAAAAAAA, 555},

    {0xffffffff, -1}
};

/* 
 *\brief translate the internal error code into CLI error code
 */
int return_cli_error_code(unsigned int error_code)
{
    int ecli = -1;
    int i = 0;
    
    if (error_code >= (unsigned int)(0xA0000000)) {
        do {
            if (code_map[i].module_code == error_code) {
                ecli = code_map[i].cli_code;
                break;
            }
            i++;
        }while (code_map[i].cli_code != -1);
    } else {
        return error_code;
    }

    return ecli;
}

/*!
 * \brief Output testing message
 * \param str_out  output string
 * \param err_code the error code 
 */
void output_test_msg(char *str_out, unsigned int err_code)
{
    int i = 0;
    int count = 0;
    int lines = 0;
    int msgs_num = MSG_WIDTH;
    char out_msg[MSG_WIDTH] = "";

    count = strlen(str_out);
    sprintf(out_msg, "%s   ", str_out);
    lines = msgs_num - count - 3;
    for (i=0; i<lines; i++)
        out_msg[count+3+i] = '-';
    out_msg[msgs_num-10] = '\0';

    lprintf(LOG_INFO, "%s", out_msg);

    if (err_code == ERR_SUCCESS)
    //    lprintf(LOG_INFO, "                                        [ \033[32m SUCCESSFUL\033[0m  ] \n");
        lprintf(LOG_INFO, "                                        [  SUCCESSFUL ] \n");
    else {
        memset(out_msg, '\0', sizeof(out_msg));
   //     lprintf(LOG_INFO, "                                        [ \033[31m FAILURE\033[0m  ]  \n");
        lprintf(LOG_INFO, "                                        [ FAILURE  ]  \n");
        sprintf(out_msg, "Error Code: [%d]\n", err_code);
        lprintf(LOG_INFO, "%s", out_msg);
    }

    return;
}

/*
 * Usage output for branched programs
 */
static void testMoBoMemory_usage(const char *progname)
{
//    fprintf(stdout, "\n");
//    fprintf(stdout, "%s %s\n", progname, VER_STR);
    fprintf(stdout, "usage: %s [-m testmode][-d cpu] [-t testitems] [-r range] [-E]\n", progname);
    fprintf(stdout, "cp: cpu, e.g. cpu0 \n");
    fprintf(stdout, "testitems: e.g. 0,1~5,9\n");
    fprintf(stdout, "verbose: e.g. 0 for default, 1 for LOG_NOTICE, 2 for LOG_INFO\n");
    fprintf(stdout, "range: e.g. 0x0,0x100000000\n");
    fprintf(stdout, "testmode: e.g. 0 for LOCAL, 1 for REMOTE, 2 for SPECIFIC, Default for LOCAL\n");
    fprintf(stdout, "    -d cpu         specify the specific cpu core to perform testing\n");
    fprintf(stdout, "    -t testitems   specify the test items\n");
    fprintf(stdout, "    -r range       specify the physical start address and the end address for testing\n");
    fprintf(stdout, "                   if no specified address, it will test all physical addresses\n");
    fprintf(stdout, "    -p verbose     specify the LOG level for debug-mode running and verbose output\n");
    fprintf(stdout, "    -m testmode    specify the test mode\n");
    fprintf(stdout, "    -c       clear recorded MCE messages\n");
    fprintf(stdout, "    -e/E     exit when error happens, no '-e' shows that don't exit until finishing all tests\n");
    fprintf(stdout, "    -v/V     shows the utility version number\n");
    fprintf(stdout, "    -s       slow test mode, do data verification during algorithm test\n");
    fprintf(stdout, "    -l       show the system memory region information\n");
    fprintf(stdout, "    -list    shows the utility usage\n");
}

/*
 * \brief 
 * Is a digit 0~9 letter
 * \para str    the input sring to be checked
 * \para a      the legal letter a in the string
 * \para b      the legal letter b in the string
 * \return  0       the string is legal
 *          nonzero     the string is an illegal string
 */

unsigned int is_legal_str(char str[LEN_128], char a, char b)
{
    char *cptr = NULL;
    char c;
    int len = 0;
    int j = 0;

    /* Get the length  */
    for (cptr=str; *cptr; ++cptr);

    len = (int) (cptr - str);

    for (j=len-1; j>=0; --j) {
        c = str[j];
        if ((c == a) || (c ==b) || (c<='9' && c>='0'))
            ;
        else {
                lperror(LOG_ERR, "Invalid parameter, the parameter should be 1~3,5,7~9");
                return ERR_PARAM;
        }
    }
    return ERR_SUCCESS;
}


/*
 * \brief
 * Get a 64 bit hex value from a string
 * \para    str     the input string
 * \return  address  the 64-bit address from the string
 */
unsigned int get_hex_address(char str[LEN_128], UL *address)
{

    char *cptr = NULL;
    char c;
    int len =0;
    int i = 0;
    int j = 0;
    UL sysaddr = 0;
    UL nbl = 0;

    
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) 
        ;
    else {
        lperror(LOG_ERR, "The input and output addresses must be started with 0x or 0X \n");
        return ERR_PARAM;
    }
    /* Get the length of str   */
    for (cptr=str; *cptr; ++cptr);
    len = (int) (cptr - str);
    /* Skip the j letters , for example 0x  */
    for (i=0, j=len-1; j>=2; --j, i+=4) {
        c = str[j];
        if (c<='9' && c>='0')
            nbl = c - '0';
        else if (c<='f' && c>='a')
            nbl = c - 'a' + 10;
        else if (c<='F' && c>='A')
            nbl = c - 'A' + 10;
        else {
            lperror(LOG_ERR, "Error: invalid hex digit");
            return ERR_PARAM;
        }
        sysaddr |= (nbl << i);
    }

    *address = sysaddr;
    return ERR_SUCCESS;
}



/*
 *\brief
 * Parse - parse input arguments and check validity
 * @name: the program name executed
 * @argc: pointer to the argument counts
 * @argv: pointer to the argument values
 * @p:    pointer to the test parameter structure
 *
 * Description: This function will parse input arguments and fill the test
 * parameter structure, while checking validity of some arguments passed in.
 * If success, zero will be returned, otherwise an error number returned.
 */
static int parse(const char *name, int *argc, char ***argv, test_parm *p)
{
    struct option opts[] = {
        {"cpu",     1, 0, 'd'},
        {"testmode",1, 0, 't'},
        {"version", 0, 0, 'v'},
        {NULL,      0, 0, 0  },
    };
    int cpu_num = 0;
    char test_str[ITEM_COUNT][LEN_16];
    int tmp_item = 0;
    int test_n = 0;
    int test_item_num = 0;
    char str[LEN_64] = "";
    char itemarg[LEN_256] = "";
    char item_string[LEN_256] = "";
    char start_str[LEN_128] = "";
    char end_str[LEN_128] = "";
    char *tmp_p = NULL;
    char *tmp_a = NULL;
    unsigned int ret = ERR_SUCCESS;

    if(*argc == 1)
        return ERR_SUCCESS;

    if (*argc == 2 && !strcmp(*(*argv + 1), "-list")) {
        testMoBoMemory_usage(name);
        exit(ERR_SUCCESS);
    }
    
    for (;;) {
        int i, j, k;
        i = getopt_long(*argc, *argv, "?d:t:r:p:m:eElLvVcCsS", opts, NULL);
        if (i == -1) {
            break;
        }
		
        switch (i) {
            case 'd':
                strcpy(str, optarg);
				if ( strlen( optarg ) < 4 ) {
					lperror(LOG_ERR, "Invalid parameter %s for -d option\n", optarg);
					return ERR_PARAM;
				}
                if (((str[0]=='c') || (str[0]=='C')) && ((str[1]=='p') || (str[1]=='P')) && ((str[2]=='u') || (str[2]=='U'))) 
                    ;
                else {
                    lperror(LOG_ERR, "Invalid parameter %s for -d option\n", optarg);
                    return ERR_PARAM;
                }
                tmp_p = optarg+3;
                
				j = strlen( tmp_p);
				for ( k = 0; k < j; k ++ ) {
					if ( tmp_p[k] > '9' || tmp_p[k] < '0' ) {
                    	lperror(LOG_ERR, "Invalid parameter %s for -d option\n", optarg);
						return ERR_PARAM;
					}	
				}
                cpu_num = atoi(tmp_p);

                if ((cpu_num < 0) || (cpu_num >= CORE_COUNT)) {
                    lperror(LOG_ERR, "Invalid parameter\n");
                    return ERR_PARAM;
                }  
                p->cpu = cpu_num;
                break;

            case 't':
                test_n = 0;
                memset((void *)&test_str, '0', sizeof(char) * ITEM_COUNT * LEN_16);
                strcpy(itemarg, optarg);
                if (is_legal_str(itemarg, '~', ',')) {
                    lperror(LOG_ERR, "Invalid parameter %s for -t option", itemarg);
                    return ERR_PARAM;
                }
                if (strlen(itemarg) <= 2) {
                    /* only one test item   */
                    strcpy(test_str[test_n], itemarg);
                    test_n ++;
                } else {
                    /* multi test items  */
                    tmp_p = strtok(itemarg, ",");
                    while (tmp_p != NULL) {
                        strcpy(test_str[test_n], tmp_p);
                        test_n++;
                        tmp_p = strtok(NULL, ",");
                    }
                }
                test_item_num = test_n;
                for (test_n=0; test_n<test_item_num; test_n++) {
                    if (strlen(test_str[test_n]) <= 2 ){
                        tmp_item = atoi(test_str[test_n]);
                        if ((tmp_item < 0) || (tmp_item >= ITEM_COUNT)) {
                            lperror(LOG_ERR, "Invalid parameter\n");
                            lperror(LOG_ERR, "Warning: test item should be e.g. 0,4,6~8\n");
                            return ERR_PARAM;
                        } else {
                            p->test_item[test_n][0] = tmp_item;
                        }
                    } else {
                        strcpy(item_string, test_str[test_n]);
                        tmp_a = strtok(item_string, "~");
                        if (tmp_a != NULL) {
                            tmp_item = atoi(tmp_a);
                            if ((tmp_item < 0) || (tmp_item >= ITEM_COUNT)) {
                                lperror(LOG_ERR, "Invalid parameter\n");
                                lperror(LOG_ERR, "Warning: test item should be e.g. 0,4,6~8\n");
                                return ERR_PARAM;
                            } else {
                                p->test_item[test_n][0] = tmp_item;
                            }
                            tmp_a = strtok(NULL, "~");
                            if (tmp_a != NULL) {
                                tmp_item = atoi(tmp_a);
                                if ((tmp_item < 0) || (tmp_item >= ITEM_COUNT)) {
                                    lperror(LOG_ERR, "Invalid parameter\n");
                                    lperror(LOG_ERR, "Warning: test item should be e.g. 0,4,6~8\n");
                                    return ERR_PARAM;
                                } else {
                                    p->test_item[test_n][1] = tmp_item;
                                }
                            }
                        }
                    }
                }
                break;

            case 'r':
                strcpy(itemarg, optarg);
                if (strlen(itemarg) < 3) {
                    lperror(LOG_ERR, "Invalid parameter\n");
                    lperror(LOG_ERR, "Warning: test start and end address should be e.g. -r 0x1000,0x100000000\n");
                    return ERR_PARAM;
                }
                tmp_p = strtok(itemarg, ",");
                if (tmp_p != NULL) {
                    strcpy(start_str, tmp_p);
                    tmp_p = strtok(NULL, ",");
                    if (tmp_p != NULL) {
                    strcpy(end_str, tmp_p);
                    }
                }
                /* Get a 64-bit hex value from string   */
                ret = get_hex_address(start_str, &(p->start));
                if (ret != ERR_SUCCESS)
                    return ret;
                ret = get_hex_address(end_str, &(p->end));
                if (ret != ERR_SUCCESS)
                    return ret;
                break;

            case 'v':
            case 'V':
                fprintf(stdout, "%s %s\n", name, VER_STR);
                exit(ERR_SUCCESS);
       
            case 'P':
	    case 'p':
                if( CheckAllDigits(optarg) == 0 ) {
                    lperror(LOG_ERR, "Invalid parameter %s for -p\n", optarg);
                    testMoBoMemory_usage(name);
                    
                    return ERR_PARAM;
                }
                p->verbose = atoi(optarg);
                switch (p->verbose) {
                    case 0:
                        p->verbose = 0;
                        break;
                    case 1:
                        p->verbose = 1;
                        break;
                    case 2:
                        p->verbose = 2;
                        break;
                    default:
                        lperror(LOG_ERR, "Invalid parameter");
                        return ERR_PARAM;
                        break;
                }
                break;
            case 'm':
                
                if( CheckAllDigits(optarg) == 0 ) {
                    lperror(LOG_ERR, "Invalid parameter %s for -m\n", optarg);
                    testMoBoMemory_usage(name);
             
                    return ERR_PARAM;
                }
                tmp_item = atoi(optarg);
                switch (tmp_item) {
                    case 0:
                        p->mode = LOCAL;
                        break;
                    case 1:
                        p->mode = REMOTE;
                        break;
                    case 2:
                        p->mode = SPECIFIC;
                        break;
                    default:
                        p->mode = LOCAL;
                        lperror(LOG_ERR, "Invalid parameter");
                        return ERR_PARAM;
                        break;
                }
                break;

            case 'E':
            case 'e':
                p->exit_error = EXIT_ON_ERROR;
                break;
            case 'L':
            case 'l':
                p->list_info = LIST_INFO;
                break;
  	    case 'C':
	    case 'c':
		p->clear_ecc = CLEAR_ECC;
		break;
            case 's':
            case 'S':
                p->no_comp = HAS_COMPARISION;
                break;

            case '?':
            default:
                p->cpu = NO_SPECIFY_CPU;
                p->mode = LOCAL;
                testMoBoMemory_usage(name);
                return ERR_USE;
      }
    }
	
	if( optind != *argc ) {
		printf("Error: Some parameters can not be recognized\n");
		testMoBoMemory_usage(name);
		return ERR_USE;
	}

    return ERR_SUCCESS;
}


/*
 *  MAIN FUNCTION DEFINITIONS
 */

/*!
 * \brief Main funcitn of the module
 * 
 * \param input
 *     argc - the parameter number
 *     argv - the parameter list
 * \param Output
 *     None
 * \return
 *    0	       - success
 *    err_code - error code if failed
 */
int main(int argc, char *argv[])
{
    int verbose = 0;
    unsigned int ret = ERR_SUCCESS;
    unsigned int cli_ret = ERR_SUCCESS;
    time_t now;
    struct tm *timenow;
    const char *progname;
    test_parm test_p;

    /* Initialize the parameters */
    memset((void *)&test_p, 0, sizeof(test_p));
    test_p.cpu = NO_SPECIFY_CPU;
    memset((void *)&(test_p.test_item), -1, sizeof(test_p.test_item));
    test_p.start = 0;
    test_p.end = 0xffffffffffff;
	test_p.exit_error = NO_EXIT_ERROR;
    test_p.verbose = 0;


    progname = (const char *)strrchr(argv[0], '/');
    progname = progname ? (progname + 1) : argv[0];

    if( CheckInstance(progname) == 1 ) {
        lprintf(LOG_ERR, "Only one instance can run on the system, quit!\n");
        return -1;
    }

    /* setup log  */
    log_init(progname, 0, verbose);

    ret = parse(progname, &argc, &argv, &test_p);
    if (ret != ERR_SUCCESS) {
        cli_ret = return_cli_error_code(ret);
        return cli_ret;
    }

    log_halt();

	verbose = test_p.verbose;

	log_init(progname, 0, verbose);

    if (getuid() != 0) {
        lperror(LOG_ERR, "Warning. Only root can excute the program!\n");
        cli_ret = return_cli_error_code(ERR_NO_PRIV);
        return cli_ret;
    }


    time(&now);
    timenow = localtime(&now);
    lprintf_n(LOG_INFO, "The start test time is %s", asctime(timenow));

    ret = init_module(MEMORY_MODULE, &test_p, 0);
    if (ret == ERR_SUCCESS) {
        cli_ret = return_cli_error_code(ret);
        output_test_msg("Initialize memory module", cli_ret);
    }
    else {
        cli_ret = return_cli_error_code(ret);
        output_test_msg("Initialize memory module", cli_ret);
        fprintf(stdout, "FAIL\n");
        //close_module(MEMORY_MODULE, &test_p, 0);
        return cli_ret;
    }

    //sleep(10);
    //exit(0);
    ret = module_test(MEMORY_MODULE, &test_p, 0);
    if (ret == ERR_SUCCESS) {
        cli_ret = return_cli_error_code(ret);
        output_test_msg("Memory module test", cli_ret);
    }else {
        cli_ret = return_cli_error_code(ret);
        output_test_msg("Memory module test", cli_ret);
        fprintf(stdout, "FAIL\n");
        return cli_ret;
    }

#if 0
    if (test_p.test_item == NO_SPECIFY_ITEM) {
        for (i=0; i<MAX_ITEM_NUM; i++) {
            time(&now);
            timenow = localtime(&now);
            D_printf("                         %s", asctime(timenow));
            ret = module_test(MEMORY_MODULE, i+1, &test_p, 0);
            if (ret == ERR_SUCCESS) {
                cli_ret = return_cli_error_code(ret);
                output_test_msg(test_items[i], cli_ret);
            }else {
                cli_ret = return_cli_error_code(ret);
                output_test_msg(test_items[i], cli_ret);
                fprintf(stdout, "FAIL\n");
                return cli_ret;
            }   
        }    
    } 
    else {
            time(&now);
            timenow = localtime(&now);
            D_printf("                         %s", asctime(timenow));

        i = test_p.test_item;
        ret = module_test(MEMORY_MODULE, i+1, &test_p, 0);
        if (ret == ERR_SUCCESS) {
            cli_ret = return_cli_error_code(ret);
            output_test_msg(test_items[i], cli_ret);
        }else {
            cli_ret = return_cli_error_code(ret);
            output_test_msg(test_items[i], cli_ret);
            fprintf(stdout, "FAIL\n");
            return cli_ret;
        }
    }
#endif
    
    ret = close_module(MEMORY_MODULE, &test_p, 0);
    if (ret == ERR_SUCCESS) {
        cli_ret = return_cli_error_code(ret);
        output_test_msg("Close memory module", cli_ret);
    }
    else {
        cli_ret = return_cli_error_code(ret);
        output_test_msg("Close memory module", cli_ret);
        fprintf(stdout, "FAIL\n");
        return cli_ret;
    }

    time(&now);
    timenow = localtime(&now);
    lprintf_n(LOG_INFO, "Finished test time is %s", asctime(timenow));
    fprintf(stdout, "PASS\n");

    /* halt log  */
    log_halt();
    return cli_ret;

}

