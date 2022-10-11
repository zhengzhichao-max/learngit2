/*------------------------------------------------------------------------*/
/* Unicode - OEM code bidirectional converter  (C)ChaN, 2009              */
/*                                                                        */
/* CP936 (Simplified Chinese GBK)                                         */
/*------------------------------------------------------------------------*/

#include "include_all.h"
#include "ff.h"




#ifdef  Simplified_Chinese  
#include "fatfs_fname_gbk.h"




#if !_USE_LFN || _CODE_PAGE != 936
#error This file is not needed in current configuration. Remove from the project.
#endif




//static fatfs_gbk_struct  fatfs_gbk;




//≥ı ºªØ
void fatfs_fname_gbk_init(fatfs_gbk_struct src)
{
    //fatfs_gbk = src;
}







#endif




