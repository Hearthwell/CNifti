#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "cnifti.h"

#include "stb_image_write.h"

#define TOK2STR(x) #x
#define NFITI_EHDR_STRING "n+1"

static const char * nifti_dt2str(uint16_t datatype){
    #define DT_CASE(x) if(datatype == x) return TOK2STR(x)
    DT_CASE(NT_UNKNOWN);
    DT_CASE(NT_BOOL);
    DT_CASE(NT_UCHAR);
    DT_CASE(NT_SHORT);
    DT_CASE(NT_INT);
    DT_CASE(NT_FLOAT);
    DT_CASE(NT_COMPLEX);
    DT_CASE(NT_DOUBLE);
    DT_CASE(NT_RGB);
    DT_CASE(NT_ALL);
    DT_CASE(NT_CHAR);
    DT_CASE(NT_USHORT);
    DT_CASE(NT_UINT);
    DT_CASE(NT_LONG_LONG);
    DT_CASE(NT_ULONG_LONG);
    DT_CASE(NT_LONG_DOUBLE);
    DT_CASE(NT_DOUBLE_PAIR);
    DT_CASE(NT_LONG_DOUBLE_PAIR);
    DT_CASE(NT_RGBA);
    printf("COULD NOT FIND STRING DATATYPE FOR %d\n", datatype);
    return "\0";
}

static const char * nifti_unit2str(uint8_t value){
    #define UNIT_CASE(x) if(value == x) return TOK2STR(x)
    UNIT_CASE(NT_UNIT_UKNOWN);
    UNIT_CASE(NT_UNIT_METER);
    UNIT_CASE(NT_UNIT_MM);
    UNIT_CASE(NT_UNIT_UM);
    UNIT_CASE(NT_UNIT_SEC);
    UNIT_CASE(NT_UNIT_MS);
    UNIT_CASE(NT_UNIT_US);
    UNIT_CASE(NT_UNIT_HZ);
    UNIT_CASE(NT_UNIT_PPM);
    UNIT_CASE(NT_UNIT_RAD_S);
    printf("COULD NOT FIND STRING UNIT FOR %d\n", value);
    return "\0";
}

static int nifti_dt2size(enum NiftiDataType dt){
    #define DT_SIZE_CASE(_type, _size) if(dt == _type) return _size
    DT_SIZE_CASE(NT_UNKNOWN, CNIFTI_DATATYPE_ERROR);
    DT_SIZE_CASE(NT_BOOL, 1);
    DT_SIZE_CASE(NT_SHORT, 2);
    DT_SIZE_CASE(NT_INT, 4);
    DT_SIZE_CASE(NT_FLOAT, 4);
    DT_SIZE_CASE(NT_COMPLEX, 8);
    DT_SIZE_CASE(NT_DOUBLE, 8);
    DT_SIZE_CASE(NT_RGB, 3);
    DT_SIZE_CASE(NT_ALL, CNIFTI_DATATYPE_ERROR);
    DT_SIZE_CASE(NT_CHAR, 1);
    DT_SIZE_CASE(NT_USHORT, 2);
    DT_SIZE_CASE(NT_UINT, 4);
    DT_SIZE_CASE(NT_LONG_LONG, 8);
    DT_SIZE_CASE(NT_ULONG_LONG, 8);
    DT_SIZE_CASE(NT_LONG_DOUBLE, 16);
    DT_SIZE_CASE(NT_DOUBLE_PAIR, 16);
    DT_SIZE_CASE(NT_LONG_DOUBLE_PAIR, 32);
    DT_SIZE_CASE(NT_RGBA, 4);
    printf("COULD NOT FIND BYTE SIZE FOR TYPE %d\n", dt);
    return CNIFTI_DATATYPE_ERROR;
}

static float nifti_get_val_float(enum NiftiDataType dt, void *val){
    #define CONVERSION_CASE(_dt, _type) if(dt == _dt) return (float)(((_type*)val)[0])
    if(dt == NT_UNKNOWN || dt == NT_BOOL || dt == NT_COMPLEX || dt == NT_RGB || dt == NT_ALL
        || dt == NT_LONG_DOUBLE || dt == NT_DOUBLE_PAIR || dt == NT_LONG_DOUBLE_PAIR){
        printf("UNSUPPORTED TYPE FOR AS FLOAT CONVERSION\n");
        return 0;
    } 
    CONVERSION_CASE(NT_UCHAR, uint8_t);
    CONVERSION_CASE(NT_SHORT, int16_t);
    CONVERSION_CASE(NT_INT, int32_t);
    CONVERSION_CASE(NT_FLOAT, float);
    CONVERSION_CASE(NT_DOUBLE, double);
    CONVERSION_CASE(NT_CHAR, char);
    CONVERSION_CASE(NT_USHORT, uint16_t);
    CONVERSION_CASE(NT_UINT, uint32_t);
    CONVERSION_CASE(NT_LONG_LONG, long long);
    CONVERSION_CASE(NT_ULONG_LONG, unsigned long long);
    CONVERSION_CASE(NT_RGBA, uint32_t);
    printf("COULD NOT GET AS FLOAT VALUE FOR TYPE %d, UNKNOWN TYPE\n", dt);
    return 0;
}

int cnifti_load(const char *path, struct NiftiImage *img){
    int fd = open(path, O_RDONLY);
    if(fd < 0){
        printf("COULD NOT OPEN FILE: %s\n", path);
        return CNIFTI_FILE_ERROR;
    }
    int status = read(fd, &img->hdr, sizeof(struct NiftiHeader));
    if(status < (int)sizeof(struct NiftiHeader)){
        printf("COULD NOT READ ENTIRE HEADER FROM NIFTI FILE\n");
        return CNIFTI_FILE_ERROR;
    }

    if(memcmp(img->hdr.mstring, NFITI_EHDR_STRING, sizeof(NFITI_EHDR_STRING))){
        printf("ERROR WITH NIFTI FILE, WRONG END OF HEADER STRING ENCOUTERED: %s\n", img->hdr.mstring);
        return CNIFTI_FILE_ERROR;
    }

    const int dt_size = nifti_dt2size(img->hdr.datatype);
    if(dt_size == CNIFTI_DATATYPE_ERROR) return CNIFTI_DATATYPE_ERROR;

    img->slice_bsize = dt_size;
    for(unsigned int i = 0; i < (unsigned int)(img->hdr.dim[0] - 1); i++) 
        img->slice_bsize *= img->hdr.dim[i + 1];
    
    const unsigned int size = img->slice_bsize * img->hdr.dim[img->hdr.dim[0]];
    img->data = malloc(size);

    lseek(fd, (long)img->hdr.vox_offset, SEEK_SET);
    status = read(fd, img->data, size);
    if(status < (int)size){
        printf("COULD NOT READ ENTIRE IMAGE DATA, GOT: %d\n", status);
        return CNIFTI_FILE_ERROR;
    }

    printf("FINISHED OPENING NIFTI FILE: %s\n", path);
    close(fd);
    return 0;
}

int cnifti_load_as_float(const char *path, struct NiftiImage *img){
    int status;
    if((status = cnifti_load(path, img)) != 0) return status;
    uint8_t *previous = img->data;
    unsigned int size = 1;
    for(unsigned int i = 0; i < img->hdr.dim[0]; i++) size *= img->hdr.dim[i + 1];
    img->data = malloc(size * sizeof(float));
    const unsigned int dt_size = nifti_dt2size(img->hdr.datatype);
    for(unsigned int i = 0; i < size; i++)
        ((float*)img->data)[i] = nifti_get_val_float(img->hdr.datatype, ((uint8_t*)previous) + dt_size * i);
    img->hdr.datatype = NT_FLOAT;
    img->slice_bsize = size * sizeof(float) / img->hdr.dim[img->hdr.dim[0]];
    free(previous);
    return 0;
}

void cnifti_print(const struct NiftiImage *nifti){
    printf("######### NIFTI HEADER #######\n");
    printf("header size: %d\n", nifti->hdr.hdr_size);
    printf("dim info: %d\n", nifti->hdr.dim_info);
    printf("ndims: %d, dims: [%d, %d, %d, %d, %d, %d, %d]\n", nifti->hdr.dim[0], nifti->hdr.dim[1], 
        nifti->hdr.dim[2], nifti->hdr.dim[3], nifti->hdr.dim[4], nifti->hdr.dim[5], nifti->hdr.dim[6], nifti->hdr.dim[7]);
    /* Intent Not Handled */
    printf("datatype: %s\n", nifti_dt2str(nifti->hdr.datatype));
    printf("slice code: %d\n", nifti->hdr.slice_code);
    printf("slice start: %d, slice end: %d\n", nifti->hdr.slice_start, nifti->hdr.slice_end);
    printf("pixdim: %f, [%f, %f, %f, %f, %f, %f, %f]\n", nifti->hdr.pixdim[0], nifti->hdr.pixdim[1],
        nifti->hdr.pixdim[2], nifti->hdr.pixdim[3], nifti->hdr.pixdim[4], nifti->hdr.pixdim[5], nifti->hdr.pixdim[6], nifti->hdr.pixdim[7]);
    printf("Voxel offset: %f\n", nifti->hdr.vox_offset);
    printf("Measurements Units: SPACIAL: %s, TEMPORAL: %s\n", 
        nifti_unit2str(nifti->hdr.xyzt_units & 0x07), nifti_unit2str(nifti->hdr.xyzt_units & 0x38));
    printf("Description: %s\n", nifti->hdr.description);
    printf("Auxilary filename: %s\n", nifti->hdr.aux_file);
    printf("Affine Transform: \n");
    printf("[%.3f, %.3f, %.3f, %.3f]\n", nifti->hdr.srow_x[0], nifti->hdr.srow_x[1], nifti->hdr.srow_x[2], nifti->hdr.srow_x[3]);
    printf("[%.3f, %.3f, %.3f, %.3f]\n", nifti->hdr.srow_y[0], nifti->hdr.srow_y[1], nifti->hdr.srow_y[2], nifti->hdr.srow_y[3]);
    printf("[%.3f, %.3f, %.3f, %.3f]\n", nifti->hdr.srow_z[0], nifti->hdr.srow_z[1], nifti->hdr.srow_z[2], nifti->hdr.srow_z[3]);
    printf("[0.000, 0.000, 0.000, 1]\n");
    printf("End hdr string: %s\n", nifti->hdr.mstring);
}

/* FORKS ONLY FOR DATATYPES WHICH THE CONVERSION TO FLOAT IS IMPLEMENTED */
struct NiftiImageMetrics cnifiti_compute_metrics(const struct NiftiImage *nifti){
    struct NiftiImageMetrics metrics = {.mean = 0.f, .std = 0.f};
    const unsigned int dt_size = nifti_dt2size(nifti->hdr.datatype);
    unsigned int size = 1;
    for(unsigned int i = 0; i < nifti->hdr.dim[0]; i++) size *= nifti->hdr.dim[i + 1];
    for(unsigned int i = 0; i < size; i++){
        float current = nifti_get_val_float(nifti->hdr.datatype, nifti->data + i * dt_size);
        metrics.mean += current / (float)size;
        metrics.std += (current * current) / (float)size;
    }
    metrics.std -= metrics.mean * metrics.mean;
    metrics.std = sqrtf(metrics.std);
    return metrics;
}

struct Nifti2DSlice cnifti_slice(struct NiftiImage *nifti, unsigned int zIndex){
    if(zIndex > nifti->hdr.dim[3]) return (struct Nifti2DSlice){}; 
    struct Nifti2DSlice slice = {
        .dt = (enum NiftiDataType) nifti->hdr.datatype,
        .width = nifti->hdr.dim[1],
        .height = nifti->hdr.dim[2],
        .xSpacing = nifti->hdr.pixdim[1],
        .ySpacing = nifti->hdr.pixdim[2],
        .zIndex = zIndex,
        .values = ((uint8_t*)nifti->data) + nifti->slice_bsize * zIndex,
        .reference = true
    };
    return slice;
}

struct Nifti2DSlice cnifti_slice_copy(const struct Nifti2DSlice *slice){
    struct Nifti2DSlice copy;
    memcpy(&copy, slice, sizeof(struct Nifti2DSlice));
    copy.reference = false;
    const unsigned int size = slice->width * slice->height * nifti_dt2size(slice->dt);
    copy.values = malloc(size);
    memcpy(copy.values, slice->values, size);
    return copy;
}

struct Nifti2DSlice cnifti_copy_slice_as_float(const struct Nifti2DSlice *slice){
    struct Nifti2DSlice result;
    memcpy(&result, slice, sizeof(struct Nifti2DSlice));
    result.reference = false;
    void *values = slice->values;
    const unsigned int size = slice->width * slice->height * sizeof(float);
    result.values = malloc(size);
    const int dt_size = nifti_dt2size(slice->dt);
    for(unsigned int i = 0; i < slice->width * slice->height; i++){
        ((float*)result.values)[i] = nifti_get_val_float(slice->dt, ((uint8_t*)values) + dt_size * i);
    }
    result.dt = NT_FLOAT;
    return result;
}

int cnifti_export_slice_img(const char *path, const struct Nifti2DSlice *slice){
    if(slice->dt != NT_FLOAT){
        printf("CNIFTI EXPORT SLICE AS JPG REQUIRES THE DATA TO BE IN FLOAT FORMAT, CONVERT USING cnifti_copy_slice_as_float\n");
        return CNIFTI_DATATYPE_ERROR;
    }
    /* TODO, MAYBE NORMALIZE USING (val - mean / std) INSTEAD OF MAX AND MIN */
    const unsigned int num_pixels = slice->width * slice->height;
    float *values = slice->values;
    float max, min;
    NIFTI_MAX(float, values, num_pixels, &max);
    NIFTI_MIN(float, values, num_pixels, &min);
    uint8_t buffer[num_pixels];
    for(unsigned int i = 0; i < num_pixels; i++){
        buffer[i] = (uint8_t)(((float)(values[i] - min) / (max - min)) * 255.0f);
    }
    stbi_write_jpg(path, slice->width, slice->height, 1, buffer, 100);
    return 0;
}

void cnifti_slice_free(struct Nifti2DSlice *slice){
    if(!slice->reference) free(slice->values);
    slice->values = NULL;
}

void cnifti_free(struct NiftiImage *img){
    free(img->data);
    img->data = NULL;
}