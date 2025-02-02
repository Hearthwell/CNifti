#include <stdio.h>
#include <string.h>

#include "cnifti.h"
#include "common.h"

#define NIFTI_IMG_PATH "test/data/s0001/mri.nii"

int main(){

    struct NiftiImage img;
    if(cnifti_load(NIFTI_IMG_PATH, &img)) return 1;

    cnifti_print(&img);
    struct Nifti2DSlice slice = cnifti_slice(&img, img.hdr.dim[3] / 2);
    struct Nifti2DSlice copy = cnifti_copy_slice_as_float(&slice);
    
    cnifti_export_slice_img("test/out/output.png", &copy);
    printf("FINISHED IMAGE OUTPUT OF SLICE\n");

    cnifti_slice_free(&copy);
    cnifti_free(&img);
    return 0;
}