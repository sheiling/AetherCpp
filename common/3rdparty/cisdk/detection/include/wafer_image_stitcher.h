#ifndef ____WAFER_IMAGE_STITCHER_H__
#define ____WAFER_IMAGE_STITCHER_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "wafer_image_stitcher_errors.h"
#include "wafer_image_stitcher_data.h"
#include "algo_mat.h"
#include "spatial_fusion_cell_locator_data.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : 
    @Modified : haopanxia@2024/04/02
    ************************************/
    C_API AlgoHandle C_CALL WaferImageStitchCreateHandle();


	C_API AlgoError C_CALL WaferImageStitchSetZoomInfo(AlgoHandle wafer_handle, const int *in_zoom_scale);



	C_API AlgoError C_CALL WaferImageStitchGetZoomInfo(AlgoHandle wafer_handle, int *out_zoom_scale);








	C_API AlgoError C_CALL WaferImageStitchSpaceConvert(AlgoHandle wafer_handle,
		const int *in_row_idx, const int *in_col_idx,
		const AlgoMat* in_img, AlgoMat *out_trans_img);










	C_API AlgoError C_CALL WaferImageStitcher(AlgoHandle wafer_handle,
		const int *in_result_num, AlgoMat **in_img_group);








	C_API AlgoError C_CALL WaferImageStitchGetImage(AlgoHandle wafer_handle, AlgoMat *out_img);








	C_API AlgoError C_CALL WaferImageStitchExportResultPng(AlgoHandle wafer_handle, const char *out_png_filename);



    /************************************
    @brief    : 获取buffer大小
    @note     :
    @param    : wafer_handle              有效的wafer句柄
    @param    : out_len                   buffer大小
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferImageGetBufferSize(AlgoHandle wafer_handle, uint64_t* out_len);




    /************************************
    @brief    : 将wafer_handle里面的数据导出到buffer，把详细的东西写清楚
    @note     :
    @param    : wafer_handle        有效的wafer句柄
    @param    : out_buffer          已经申请好申请空间
    @param    : in_out_len          外部申请好的空间大小,会用真实大小修改
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferImageToBuffer(AlgoHandle wafer_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief    : 将buffer数据导入到WaferDetect
    @note     :
    @param    : wafer_handle              有效的wafer句柄
    @param    : in_buffer
    @param    : in_len
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferImageFromBuffer(AlgoHandle wafer_handle, const char* in_buffer, uint64_t in_len);



    /************************************
    Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
    Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr;

    handle=new OBJ;
    OBJ.Set(a_lot_data);

    Release 相当于:
        OBJ.Release();
    Destroy 相当于:
        OBJ.release();
        delete handle;
    ************************************/
    C_API AlgoError C_CALL WaferImageRelease(AlgoHandle wafer_handle);
    C_API void C_CALL WaferImageDestroyHandle(AlgoHandle wafer_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif