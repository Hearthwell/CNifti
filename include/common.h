#ifndef CNIFTI_COMMON_H
#define CNIFTI_COMMON_H

#define CNIFTI_SIMPLE_ERROR -1
#define CNIFTI_FILE_ERROR -2
#define CNIFTI_DATATYPE_ERROR -3

#define NIFTI_MAX(_type, _ptr, _size, _dst)\
do{ \
    _type *__values = (_type *)_ptr; \
    _type __max = __values[0]; \
    for(unsigned int i = 1; i < _size; i++){ \
        if(__values[i] > __max) __max = __values[i]; \
    } \
    *_dst = __max; \
}while(false) \

#define NIFTI_MIN(_type, _ptr, _size, _dst)\
do{ \
    _type *__values = (_type *)_ptr; \
    _type __min = __values[0]; \
    for(unsigned int i = 1; i < _size; i++){ \
        if(__values[i] < __min) __min = __values[i]; \
    } \
    *_dst = __min; \
}while(false) \

#endif //CNIFTI_COMMON_H
