/*!
*\file memory_err.c
* 
* Define functions for operating abs_diag proc file 
*
*\author Maggie Tong
*
* Copyright(c) 2009 Jabil Circuit.
*
* This source code and any compilation or derivative thereof is the sole property of 
* Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
* is the proprietary information of Jabil Circuit and is confidential in nature. 
* It's use and dissemination by any party other than Jabil Circuit is strictly 
* limited by the confidential information provisions of Software License Agreement 
* referenced above.
*
* Version history
* Nov. 17, 2009    Create this file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jedi_comm.h"
#include "memory_err.h"

char error_info[LEN_1024] = "";

/*!
 * Output the error message
 *
 *\param	input	err_no - the error number
 *
 *\param	return  None      
 */
void memory_error_msg(unsigned int err_no)
{
    char err_msg[LEN_256];

    memset(err_msg, 0, sizeof(err_msg));
    sprintf(err_msg, "%s %s: ", __DATE__, __TIME__);


/*  D_printf("Error [%s](%s %s)\n", error_info, __DATE__, __TIME__); */

    switch (err_no){
        case ERR_DD_FAIL:
            D_printf("Fail to operate absdiag_dev proc file\n");
            strcat(err_msg, "Fail to operate absdiag_dev proc file\n");
            break;
        case ERR_DD_OPEN:
            D_printf("Fail to open absdiag_dev proc file without the abs_diag.ko module\n");
            strcat(err_msg, "Fail to open absdiag_dev proc file\n");
            break;
        case ERR_NO_ABSDIAG_DEV_PROC_FILE:
            D_printf("No absdiag device proc file\n");
            strcat(err_msg, "No absdiag device proc file\n");
            break;
        case ERR_SET_CPU_AFFINITY:
            D_printf("Fail to set CPU Affinity\n");
            strcat(err_msg, "Fail to set CPU Affinity\n");
            break;
        case ERR_CPU_NOT_SUPPORT_CPUID:
            D_printf("Fail for not supporting CPUID instrucion\n");
            strcat(err_msg, "Fail for not supporting CPUID instrucion\n");
            break;
        case ERR_TEST_UNIT_NUM:
            D_printf("Fail for the error test unit number!\n");
            strcat(err_msg, "Fail for the error test unit number!\n");
            break;
        case ERR_WRITE_DD_FILENAME:
            D_printf("Fail to write the DD file!\n");
            strcat(err_msg, "Fail to write the DD file!\n");
            break;
    
        case ERR_MODULE_ID:
            D_printf("The error module ID!\n");
            strcat(err_msg, "The error module ID!\n");
            break;
    
        case ERR_ITEM_ID:
            D_printf("The error item ID!\n");
            strcat(err_msg, "The error item ID!\n");
            break;
    
        case ERR_GET_MEMORY_INFO:
            D_printf("Fail to get memory info.!\n");
            strcat(err_msg, "Fail to get memory info.!\n");
            break;
    
        case ERR_OPEN_TEMP_FILE:
            D_printf("Fail to open the temp file!\n");
            strcat(err_msg, "Fail to open the temp file!\n");
            break;
 
        case ERR_READ_TEMP_FILE:
            D_printf("Fail to read the temp file!\n");
            strcat(err_msg, "Fail to read the temp file!\n");
            break;
 
        case ERR_CORE_NUM:
            D_printf("The error cpu-core number!\n");
            strcat(err_msg, "The error cpu-core number!\n");
            break;
 
        case ERR_MALLOC_SPACE:
            D_printf("Fail to malloc system call!\n");
            strcat(err_msg, "Fail to malloc system call!\n");
            break;

        case ERR_TEST_ITEM_NUM:
            D_printf("The error test item number!\n");
            strcat(err_msg, "The error test item number!\n");
            break;
 
        case ERR_MLOCK_SPACE:
            D_printf("Fail to mlock system call!\n");
            strcat(err_msg, "Fail to mlock system call!\n");
            break;
 
        case ERR_CREATE_THREAD:
            D_printf("Fail to create the thread!\n");
            strcat(err_msg, "Fail to create the thread!\n");
            break;
 
        case ERR_MUNLOCK_SPACE:
            D_printf("Fail to nunlock system call!\n");
            strcat(err_msg, "Fail to nunlock system call!\n");
            break;
 
        case ERR_BIND_CPU:
            D_printf("Fail to bind cpu core!\n");
            strcat(err_msg, "Fail to bind cpu core!\n");
            break;
 
        case ERR_ADDR_WALK_ONES:
            D_printf("Fail to test address walking ones !\n");
            strcat(err_msg, "Fail to test address walking ones !\n");
            break;

        case ERR_NO_ENOUGH_MEM_SPACE:
            D_printf("Failure for no enough mem space!\n");
            strcat(err_msg, "Failure for no enough mem space!\n");
            break;
 
        case ERR_ADDR_WALK_OWN:
            D_printf("Fail to test address walking own address!\n");
            strcat(err_msg, "Fail to test address walking own address!\n");
            break;
 
        case ERR_MOVINV_10:
            D_printf("Fail to test moving inversion ONES&ZEROS!\n");
            strcat(err_msg, "Fail to test moving inversion ONES&ZEROS!\n");
            break;
 
        case ERR_MOVINV_8BIT:
            D_printf("Fail to test moving inversion 8BIT!\n");
            strcat(err_msg, "Fail to test moving inversion 8BIT!\n");
            break;
 
        case ERR_MOVINV_RANDOM:
            D_printf("Fail to test moving inversion random!\n");
            strcat(err_msg, "Fail to test moving inversion random!\n");
            break;
 
        case ERR_BLOCK_MOVE:
            D_printf("Fail to test block move!\n");
            strcat(err_msg, "Fail to test block move!\n");
            break;
 
        case ERR_MOVINV_32BIT:
            D_printf("Fail to test moving inversion long pattern!\n");
            strcat(err_msg, "Fail to test moving inversion long pattern!\n");
            break;
 
        case ERR_RANDOM_NUM_SEQ:
            D_printf("Fail to test random number sequence!\n");
            strcat(err_msg, "Fail to test random number sequence!\n");
            break;
 
        case ERR_MODULO:
            D_printf("Fail to test modulo-20 !\n");
            strcat(err_msg, "Fail to test modulo-20 !\n");
            break;
 
        case ERR_BIT_FADE:
            D_printf("Fail to test bit fade!\n");
            strcat(err_msg, "Fail to test bit fade!\n");
            break;
			
		case ERR_MARCH_C:
		    D_printf("Fail to test March C!\n");
			strcat(err_msg, "Fail to test March C!\n");
            break;
			
        case ERR_CACHE_WALKING:
            D_printf("Fail to test cache walking!\n");
            strcat(err_msg, "Fail to test cache walking!\n");
            break;
 
        case ERR_CACHE_WALK_1:
            D_printf("Fail to test cache walking one!\n");
            strcat(err_msg, "Fail to test cache walking one!\n");
            break;
 
        case ERR_CACHE_WALK_0:
            D_printf("Fail to test cache walking zero!\n");
            strcat(err_msg, "Fail to test cache walking zero!\n");
            break;
 
        case ERR_CACHE_STUCK:
            D_printf("Fail to test cache stuck!\n");
            strcat(err_msg, "Fail to test cache stuck!\n");
            break;
 
        case ERR_CACHE_RDWR:
            D_printf("Fail to test cache read/write!\n");
            strcat(err_msg, "Fail to test cache read/write!\n");
            break;
 
        case ERR_CACHE_RAND_DATA:
            D_printf("Fail to test cache rand data!\n");
            strcat(err_msg, "Fail to test cache rand data!\n");
            break;
 
        case ERR_TEXT_WRITE:
            D_printf("Fail to write the PROC text file!\n");
            strcat(err_msg, "Fail to write the PROC text file!\n");
            break;
        case ERR_TIMEOUT_CACHE_TEST:
            D_printf("Timeount to cache test!\n");
            strcat(err_msg, "Timeount to cache test!\n");
            break;
        case ERR_CACHE_SWITCH:
            D_printf("Fail to switch cpu cache!\n");
            strcat(err_msg, "Fail to switch cpu cache!\n");
            break;
        case ERR_TURNOFF_CACHE:
            D_printf("Failure for turning-off cpu cache\n");
            strcat(err_msg, "Failure for turning-off cpu cache\n");
            break;

        default:
            D_printf("Don't define the error code, please check it\n");
            strcat(err_msg, "Don't define the error code, please check it\n");
            break;
    }

    if (strlen(error_info)) {
        strcat(err_msg, ": ");
        strcat(err_msg, error_info);
    }

//    jedi_create_log_file(err_no, err_msg);

}

