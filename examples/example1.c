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
    struct Nifti2DSlice copy = cnifti_slice_copy(&slice);
    cnifti_slice_as_float(&copy);
    
    cnifti_export_slice_img("test/out/output.png", &copy);

    cnifti_slice_free(&copy);
    cnifti_free(&img);
    return 0;
}