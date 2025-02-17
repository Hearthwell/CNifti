#ifndef CNIFTI_H
#define CNIFTI_H

#include <stdint.h>
#include <stdbool.h>

enum NiftiDataType{
    NT_UNKNOWN          = 0,
    NT_BOOL             = 1,
    NT_UCHAR            = 2,
    NT_SHORT            = 4,
    NT_INT              = 8,
    NT_FLOAT            = 16,
    NT_COMPLEX          = 32,
    NT_DOUBLE           = 64,
    NT_RGB              = 128,
    NT_ALL              = 255,
    NT_CHAR             = 256,
    NT_USHORT           = 512,
    NT_UINT             = 768,
    NT_LONG_LONG        = 1024,
    NT_ULONG_LONG       = 1280,
    NT_LONG_DOUBLE      = 1536, /* 128 BITS */
    NT_DOUBLE_PAIR      = 1792,
    NT_LONG_DOUBLE_PAIR = 2048,
    NT_RGBA             = 2304,
};
#define NUM_NIFTI_DATATYPES 19

enum NiftiUnit{
    NT_UNIT_UKNOWN = 0,
    NT_UNIT_METER  = 1,
    NT_UNIT_MM     = 2,
    NT_UNIT_UM     = 3,
    NT_UNIT_SEC    = 8,
    NT_UNIT_MS     = 16,
    NT_UNIT_US     = 24,
    NT_UNIT_HZ     = 32,
    NT_UNIT_PPM    = 40,
    NT_UNIT_RAD_S  = 48,
};

/* ALL FIELDS STARTING BY __ IN NiftiHeader STRUCT ARE UNUSED */

struct _AnalyseCompat{
    uint8_t data_type[10];
    uint8_t db_name[18];
    uint32_t extents;
    uint16_t session_error;
    uint8_t regular;
} __attribute__((packed));

struct NiftiHeader{
    uint32_t hdr_size;
    struct _AnalyseCompat __unused;
    uint8_t dim_info;
    uint16_t dim[8];
    float intent_p1, intent_p2, intent_p3;
    uint16_t intent_code;
    uint16_t datatype;
    uint16_t bitpix;
    uint16_t slice_start;
    float pixdim[8];
    float vox_offset;
    float scl_slope;
    float scl_inter;
    uint16_t slice_end;
    uint8_t slice_code;
    uint8_t xyzt_units;
    float call_max, call_min;
    float slice_duration;
    float toffset;
    int32_t __glmax, __glmin;
    char description[80];
    char aux_file[24];
    uint16_t qform_code, sform_code;
    float quatern_b, quatern_c, quatern_d;
    float qoffset_x, qoffset_y, qoffset_z;
    float srow_x[4];
    float srow_y[4];
    float srow_z[4];
    char intent_name[16];
    char mstring[4];
};

struct NiftiImage{
    struct NiftiHeader hdr;
    unsigned int slice_bsize;
    void *data;
};

struct NiftiImageMetrics{
    float std;
    float mean;
};

struct Nifti2DSlice{
    enum NiftiDataType dt;
    unsigned int width, height;
    float xSpacing, ySpacing;
    unsigned int zIndex;
    void *values;
    bool reference;
};

int cnifti_load(const char *path, struct NiftiImage *img);
int cnifti_load_as_float(const char *path, struct NiftiImage *img);
void cnifti_print(const struct NiftiImage *nifti);
struct NiftiImageMetrics cnifiti_compute_metrics(const struct NiftiImage *nifti);
struct Nifti2DSlice cnifti_slice(struct NiftiImage *nifti, unsigned int zIndex);
struct Nifti2DSlice cnifti_slice_copy(const struct Nifti2DSlice *slice);
struct Nifti2DSlice cnifti_copy_slice_as_float(const struct Nifti2DSlice *slice);
int cnifti_export_slice_img(const char *path, const struct Nifti2DSlice *slice);

void cnifti_slice_free(struct Nifti2DSlice *slice);
void cnifti_free(struct NiftiImage *img);


#endif //CNIFTI_H
