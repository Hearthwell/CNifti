#include <stdio.h>
#include <string.h>

#include "cnifti.h"
#include "common.h"

#define NIFTI_IMG_PATH "test/data/s0001/mri.nii"

int main(){

    struct NiftiImage img;
    if(cnifti_load_as_float(NIFTI_IMG_PATH, &img)) return 1;

    cnifti_print(&img);
    /* NO NEED TO TRANSFORM SLICE TO FLOAT SINCE WE LOADED THE ENTIRE MODEL AS FLOAT */
    struct Nifti2DSlice slice = cnifti_slice(&img, img.hdr.dim[3] / 2);
    
    cnifti_export_slice_img("test/out/output2.png", &slice);
    printf("FINISHED IMAGE OUTPUT OF SLICE\n");

    cnifti_free(&img);
    return 0;
}