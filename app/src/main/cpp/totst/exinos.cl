    #pragma OPENCL EXTENSION cl_khr_fp16 : enable
    __constant sampler_t sampler =	CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP|CLK_FILTER_NEAREST;
    __kernel void compute_cell_features(__global const float*     mag,
           __global const float*      angle,
           __constant const short*   shape __attribute__((max_constant_size(452))),
           __constant const char*    lut1 __attribute__((max_constant_size(288))),
           __constant const float*    lut2 __attribute__((max_constant_size(1152))),
           __global float*            _allBlockVecs,
           float                      bin_interval,
           int                       block_vec_length,
           int                       src_width,
           int                       src_height)
    {
        const int wid_z = get_global_id(0);
        const int wid_x = get_global_id(1);
        const int wid_y = get_global_id(2);
        const int CELL_SIZE = 12;
        const int NUM_OF_ORIENTATION_BINS = 8;
        float magnitude, pixel, fraction, mw1, mw2;
        char2 xyMinus;
        float4 wxwy;
        int bin1, bin2, binx, biny, whole;
        int row = wid_y/CELL_SIZE;
        int col = wid_x/CELL_SIZE;
        int x = (int) shape[wid_z];
        int y = (int) shape[wid_z + 113];
        int offsetRow = y - CELL_SIZE + wid_y;
        int offsetCol = x - CELL_SIZE + wid_x;
        int stride = wid_z * block_vec_length;
//            int dif = - offsetCol - offsetRow*CELL_SIZE;
        int idxLUT = (wid_x - col*CELL_SIZE) + (wid_y - row*CELL_SIZE)*CELL_SIZE;
        idxLUT = idxLUT<<1;
        int idxLUT2 = idxLUT<<1;
        int idxSRC = offsetRow*src_width + offsetCol;
        int cellIndex;
                magnitude = mag[idxSRC];
                pixel = angle[idxSRC];
                bin1 = 0;
                bin2 = 0;
                if (pixel <= 0) {
                    pixel +=180;
                    bin1 += NUM_OF_ORIENTATION_BINS;
                    bin2 += NUM_OF_ORIENTATION_BINS;
                }
                pixel /= bin_interval;
                whole = pixel;
                fraction = pixel - whole;
                if (whole != NUM_OF_ORIENTATION_BINS - 1) bin1 += whole + 1;
                bin2 += whole;
                xyMinus = vload2(0, lut1 + idxLUT);
                wxwy = vload4(0, lut2 + idxLUT2);
                binx = xyMinus.s0 + col;
                biny = xyMinus.s1 + row;
                mw1 = magnitude * fraction;
                mw2 = magnitude - mw1;
                int startBin1 = bin1 * 4 + stride;
                int startBin2 = bin2 * 4 + stride;
                if(binx >= 0 && biny >= 0){
                    cellIndex = biny * 2 + binx;
                    _allBlockVecs[startBin1 + cellIndex] += mw1 * wxwy.s0;
                    _allBlockVecs[startBin2 + cellIndex] += mw2 * wxwy.s0;
                }
                if(binx < 1 && biny >= 0){
                    cellIndex = (biny * 2 + (binx + 1));
                    _allBlockVecs[startBin1 + cellIndex] += mw1 * wxwy.s1;
                    _allBlockVecs[startBin2 + cellIndex] += mw2 * wxwy.s1;
                }
                if(binx < 1 && biny < 1){
                    cellIndex = ((biny + 1) * 2 + (binx + 1));
                    _allBlockVecs[startBin1 + cellIndex] += mw1 * wxwy.s2;
                    _allBlockVecs[startBin2 + cellIndex] += mw2 * wxwy.s2;
                }
                if(binx >= 0 && biny < 1){
                    cellIndex = ((biny + 1) * 2 + binx);
                    _allBlockVecs[startBin1 + cellIndex] += mw1 * wxwy.s3;
                    _allBlockVecs[startBin2 + cellIndex] += mw2 * wxwy.s3;
                }
    }
    __kernel void normalize_vec(__global float*     _allBlockVecs)
    {
        const int wid_z = get_global_id(0);
        float16 sum;
    int start = wid_z * 100;
    int stride = 32;
    int start2 = start + stride;
    int start3 = start2 + stride;
    int start4 = start3 + stride;
    float16 h1, h2, h3, h4, h11, h21, h31, h41, hc1, hc2;
    h1 = vload16(0, _allBlockVecs + start);
    h2 = vload16(0, _allBlockVecs + start + 16);
    h3 = vload16(0, _allBlockVecs + start + 32);
    h4 = vload16(0, _allBlockVecs + start + 48);
    h11 = fabs(h1);
    h21 = fabs(h2);
    h31 = fabs(h3);
    h41 = fabs(h4);
    sum = (fmax(h11, h31) + fmax(h21, h41))*0.960433870103f + (fmin(h11, h31) + fmin(h21, h41))*0.397824734759f;
    float factor = 1.0 / (sum.s0 + sum.s1 + sum.s2 + sum.s3 + sum.s4 + sum.s5 + sum.s6 + sum.s7 + sum.s8 + sum.s9 + sum.sA + sum.sB + sum.sC + sum.sD + sum.sE + sum.sF + 1e-9);
//        float text[4] = {0.0, 0.0, 0.0, 0.0};
    float4 text1;
    h1 *= factor;
    h2 *= factor;
    h3 *= factor;
    h4 *= factor;
    hc1 = h1 + h3;
    hc2 = h2 + h4;
    text1.s0 = hc1.s0 + hc1.s4 + hc1.s8 + hc1.sC + hc2.s0 + hc2.s4 + hc2.s8 + hc2.sC;
    text1.s1 = hc1.s1 + hc1.s5 + hc1.s9 + hc1.sD + hc2.s1 + hc2.s5 + hc2.s9 + hc2.sD;
    text1.s2 = hc1.s2 + hc1.s6 + hc1.sA + hc1.sE + hc2.s2 + hc2.s6 + hc2.sA + hc2.sE;
    text1.s3 = hc1.s3 + hc1.s7 + hc1.sB + hc1.sF + hc2.s3 + hc2.s7 + hc2.sB + hc2.sF;
    text1 *= (float) 0.2357;
    vstore16(h1, 0, _allBlockVecs + start);
    vstore16(h2, 0, _allBlockVecs + start + 16);
    vstore16(h3, 0, _allBlockVecs + start + 32);
    vstore16(h4, 0, _allBlockVecs + start + 48);
    vstore16(hc1, 0, _allBlockVecs + start + 64);
    vstore16(hc2, 0, _allBlockVecs + start + 80);
    vstore4(text1, 0, _allBlockVecs + start + 96);
    }
    
    __kernel void normalize_vec2(__global float*     _allBlockVecs)
    {
        const int wid_z = get_global_id(0);
        int sum = 0;
    int start = wid_z * 100;
    int stride = 32;
    int start2 = start + stride;
    int start3 = start2 + stride;
    int start4 = start3 + stride;
    for(int i = start, i2 = start2;i < start + stride;i++, i2++)
    {
        float h1 = fabs(_allBlockVecs[i]);
        float h2 = fabs(_allBlockVecs[i2]);
        float h = fmax(h1, h2)*0.960433870103f + fmin(h1, h2)*0.397824734759f;
        sum += h;
    }
    float factor = 1.0 / (sum + 1e-9);
    float text[4] = {0.0, 0.0, 0.0, 0.0};
    for(int i = start, i2 = start2, i3 = start3; i < start + stride; ++i, ++i2, ++i3){
        float ha = _allBlockVecs[i];
        float hb = _allBlockVecs[i2];
        ha *= factor;
        hb *= factor;
        float hc = ha + hb;
        text[i % 4] += hc;
        _allBlockVecs[i]  = ha;
        _allBlockVecs[i2] = hb;
        _allBlockVecs[i3] = hc;
    }
    for(int i = 0; i < 4; i++){
        _allBlockVecs[start4 + i] = 0.2357 * text[i];
    }
    }
    
    __kernel void pyr_down_opt2(__global const uchar*  restrict   src_img,
                           __global uchar*     restrict      result_img,
                           __constant const uchar*    pyr_mat __attribute__((max_constant_size(25))),
                                   int                       src_width,
                                   int                       src_size,
                                   int                       result_width)
    {
        const int result_x = get_global_id(0)<<1;
        const int result_y = get_global_id(1)<<1;
        const int wid_x = result_x<<1;
        const int wid_y = result_y<<1;
        int idx = 0;
        int idx1 = (wid_y - 2)*src_width + wid_x - 2;
        int idx2 = idx1;
        int idx3 = result_y * result_width + result_x;
        uchar8 data[7];
        uchar2 result[2];
    #pragma unroll 
        for (int i = 0; i < 7; ++i)
        {
           data[i] = vload8(0, src_img + idx2);
           idx2+=src_width;
        }
        int result_1 = data[0].s0 * pyr_mat[0] + data[0].s1 * pyr_mat[1] + data[0].s2 * pyr_mat[2] + data[0].s3 * pyr_mat[3] + data[0].s4 * pyr_mat[4] +
                       data[1].s0 * pyr_mat[5] + data[1].s1 * pyr_mat[6] + data[1].s2 * pyr_mat[7] + data[1].s3 * pyr_mat[8] + data[1].s4 * pyr_mat[9] +
                       data[2].s0 * pyr_mat[10] + data[2].s1 * pyr_mat[11] + data[2].s2 * pyr_mat[12] + data[2].s3 * pyr_mat[13] + data[2].s4 * pyr_mat[14] +
                       data[3].s0 * pyr_mat[15] + data[3].s1 * pyr_mat[16] + data[3].s2 * pyr_mat[17] + data[3].s3 * pyr_mat[18] + data[3].s4 * pyr_mat[19] +
                       data[4].s0 * pyr_mat[20] + data[4].s1 * pyr_mat[21] + data[4].s2 * pyr_mat[22] + data[4].s3 * pyr_mat[23] + data[4].s4 * pyr_mat[24];
        int result_2 = data[0].s2 * pyr_mat[0] + data[0].s3 * pyr_mat[1] + data[0].s4 * pyr_mat[2] + data[0].s5 * pyr_mat[3] + data[0].s6 * pyr_mat[4] +
                       data[1].s2 * pyr_mat[5] + data[1].s3 * pyr_mat[6] + data[1].s4 * pyr_mat[7] + data[1].s5 * pyr_mat[8] + data[1].s6 * pyr_mat[9] +
                       data[2].s2 * pyr_mat[10] + data[2].s3 * pyr_mat[11] + data[2].s4 * pyr_mat[12] + data[2].s5 * pyr_mat[13] + data[2].s6 * pyr_mat[14] +
                       data[3].s2 * pyr_mat[15] + data[3].s3 * pyr_mat[16] + data[3].s4 * pyr_mat[17] + data[3].s5 * pyr_mat[18] + data[3].s6 * pyr_mat[19] +
                       data[4].s2 * pyr_mat[20] + data[4].s3 * pyr_mat[21] + data[4].s4 * pyr_mat[22] + data[4].s5 * pyr_mat[23] + data[4].s6 * pyr_mat[24];

        int result_3 = data[2].s0 * pyr_mat[0] + data[2].s1 * pyr_mat[1] + data[2].s2 * pyr_mat[2] + data[2].s3 * pyr_mat[3] + data[2].s4 * pyr_mat[4] +
                       data[3].s0 * pyr_mat[5] + data[3].s1 * pyr_mat[6] + data[3].s2 * pyr_mat[7] + data[3].s3 * pyr_mat[8] + data[3].s4 * pyr_mat[9] +
                       data[4].s0 * pyr_mat[10] + data[4].s1 * pyr_mat[11] + data[4].s2 * pyr_mat[12] + data[4].s3 * pyr_mat[13] + data[4].s4 * pyr_mat[14] +
                       data[5].s0 * pyr_mat[15] + data[5].s1 * pyr_mat[16] + data[5].s2 * pyr_mat[17] + data[5].s3 * pyr_mat[18] + data[5].s4 * pyr_mat[19] +
                       data[6].s0 * pyr_mat[20] + data[6].s1 * pyr_mat[21] + data[6].s2 * pyr_mat[22] + data[6].s3 * pyr_mat[23] + data[6].s4 * pyr_mat[24];
        int result_4 = data[2].s2 * pyr_mat[0] + data[2].s3 * pyr_mat[1] + data[2].s4 * pyr_mat[2] + data[2].s5 * pyr_mat[3] + data[2].s6 * pyr_mat[4] +
                       data[3].s2 * pyr_mat[5] + data[3].s3 * pyr_mat[6] + data[3].s4 * pyr_mat[7] + data[3].s5 * pyr_mat[8] + data[3].s6 * pyr_mat[9] +
                       data[4].s2 * pyr_mat[10] + data[4].s3 * pyr_mat[11] + data[4].s4 * pyr_mat[12] + data[4].s5 * pyr_mat[13] + data[4].s6 * pyr_mat[14] +
                       data[5].s2 * pyr_mat[15] + data[5].s3 * pyr_mat[16] + data[5].s4 * pyr_mat[17] + data[5].s5 * pyr_mat[18] + data[5].s6 * pyr_mat[19] +
                       data[6].s2 * pyr_mat[20] + data[6].s3 * pyr_mat[21] + data[6].s4 * pyr_mat[22] + data[6].s5 * pyr_mat[23] + data[6].s6 * pyr_mat[24];
            result[0].s0 = result_1>>8;
            result[0].s1 = result_2>>8;
            result[1].s0 = result_3>>8;
            result[1].s1 = result_4>>8;
            vstore2(result[0], 0, result_img + idx3);
            vstore2(result[1], 0, result_img + idx3 + result_width);
    }
    
 __kernel void pyr_down(__global const uchar*  restrict   src_img,
                           __global uchar*     restrict      result_img,
                           __constant const uchar*    pyr_mat __attribute__((max_constant_size(25))),
                                   int                       src_width,
                                   int                       src_size,
                                   int                       result_width)
    {
        const int result_x = get_global_id(0);
        const int result_y = get_global_id(1);
        const int wid_x = result_x<<1;
        const int wid_y = result_y<<1;
        int idx = 0;
        int idx1 = (wid_y - 2)*src_width + wid_x;
        int sum = 0;
    #pragma unroll 
        for (int i = -2; i < 3; ++i)
        {
    #pragma unroll 
           for (int j = -2; j < 3; ++j)
           {
               sum += src_img[idx1 + j]*pyr_mat[idx];
               idx++;
           }
           idx1 += src_width;
        }
        result_img[result_y*result_width + result_x] = sum>>8;
    }
    
    __kernel void histogram_adjust(__global uchar*           src_img,
                                   int                       src_width,
                                   int                       src_size,
                                   int                       hist_value)
    {
        const int wid_x = get_global_id(0);
        const int wid_y = get_global_id(1);
        int idx = wid_y*src_width + wid_x;
        if (src_img[idx] == 0) src_img[idx] = hist_value;
    }
    
    __kernel void affine_transform(__global const uchar*  restrict   src_img,
                                   __global uchar*  restrict  result_img,
                                   __constant const float*    trans_mat __attribute__((max_constant_size(24))),
                                   int                       src_width,
                                   int                       src_size)
    {
        const int wid_x = get_global_id(0);
        const int wid_y = get_global_id(1);
        uchar pixel = src_img[wid_y*src_width + wid_x];
        int2 transCoords = { wid_x*trans_mat[0] + wid_y*trans_mat[1] + trans_mat[2], wid_x*trans_mat[3] + wid_y*trans_mat[4] + trans_mat[5]};
        int idx = transCoords.y*src_width + transCoords.x;
        if (idx > 0 && idx < src_size)result_img[idx] = pixel;
    }
    
 __kernel void sobelDxDy(__global const uchar*      restrict   src_image,
                                 __global float*     restrict      result_mag,
                                 __global float*     restrict      result_angle,
                                 int                      src_width,
                                 int                      startX,
                                 int                      startY,
                                 int                      result_width)
    {
        const int wid_x = get_global_id(0);
        const int wid_y = get_global_id(1);
        const int idx = (wid_y + startY) * src_width + wid_x + startX;
        const int idx2 = wid_y * result_width + wid_x;
        float dx = src_image[idx+1] - src_image[idx - 1];
        float dy = src_image[idx+src_width] - src_image[idx - src_width];
        result_angle[idx2] = atan2(dy, dx)*57.29577951f;
        dx = fabs(dx);
        dy = fabs(dy);
        result_mag[idx2] = fmax(dx, dy)*0.960433870103f + fmin(dx, dy)*0.397824734759;
    
    }
// The remainder kernel calculates the leftover of matmul operation which couldn't fit into the matmul kernel tile
    __kernel void matmul_remainder( __global const float* restrict matrix_a,
                                   __global const  float* restrict matrix_b,
                                   __global        float* restrict matrix_c,
                                                   int    x_rem_start,
                                                   int    y_rem_start,
                                                   int    result_width,
                                                   int    matrix_a_width)
    {
        const int wid_x = get_global_id(0);
        const int wid_y = y_rem_start + get_global_id(1);
    
        float16 a, a1, a2, a3;
        float16 b, b1, b2, b3;
        int   b_idx = wid_x << 6;
        int   idx = matrix_a_width * wid_y + b_idx;
        int   idx2 = wid_y * result_width + wid_x;
        b = vload16(0, matrix_b + b_idx);
        b1 = vload16(0, matrix_b + b_idx + 16);
        b2 = vload16(0, matrix_b + b_idx + 32);
        b3 = vload16(0, matrix_b + b_idx + 48);
    
        a = vload16(0, matrix_a + idx);
        a1 = vload16(0, matrix_a + idx + 16);
        a2 = vload16(0, matrix_a + idx + 32);
        a3 = vload16(0, matrix_a + idx + 48);
        matrix_c[idx2] = a.s0*b.s0 + a.s1*b.s1 + a.s2*b.s2 + a.s3*b.s3 + a.s4*b.s4 + a.s5*b.s5 + a.s6*b.s6 + a.s7*b.s7 + a.s8*b.s8 + a.s9*b.s9 + a.sA*b.sA + a.sB*b.sB + a.sC*b.sC + a.sD*b.sD + a.sE*b.sE + a.sF*b.sF + 
                           a2.s0*b2.s0 + a2.s1*b2.s1 + a2.s2*b2.s2 + a2.s3*b2.s3 + a2.s4*b2.s4 + a2.s5*b2.s5 + a2.s6*b2.s6 + a2.s7*b2.s7 + a2.s8*b2.s8 + a2.s9*b2.s9 + a2.sA*b2.sA + a2.sB*b2.sB + a2.sC*b2.sC + a2.sD*b2.sD + a2.sE*b2.sE + a2.sF*b2.sF + 
                           a3.s0*b3.s0 + a3.s1*b3.s1 + a3.s2*b3.s2 + a3.s3*b3.s3 + a3.s4*b3.s4 + a3.s5*b3.s5 + a3.s6*b3.s6 + a3.s7*b3.s7 + a3.s8*b3.s8 + a3.s9*b3.s9 + a3.sA*b3.sA + a3.sB*b3.sB + a3.sC*b3.sC + a3.sD*b3.sD + a3.sE*b3.sE + a3.sF*b3.sF + 
                           a1.s0*b1.s0 + a1.s1*b1.s1 + a1.s2*b1.s2 + a1.s3*b1.s3 + a1.s4*b1.s4 + a1.s5*b1.s5 + a1.s6*b1.s6 + a1.s7*b1.s7 + a1.s8*b1.s8 + a1.s9*b1.s9 + a1.sA*b1.sA + a1.sB*b1.sB + a1.sC*b1.sC + a1.sD*b1.sD + a1.sE*b1.sE + a1.sF*b1.sF;
    
    }
    
// The matmul kernel calculates matrix-vector multiplication using 64x64 size tile
    __kernel void matmul( __global const  float* restrict matrix_a,
                          __global const  float* restrict matrix_b,
                          __global        float* restrict matrix_c,
                                                   int   x_rem_start,
                                                   int   y_rem_start,
                                                   int   result_width,
                                                   int   matrix_a_width)
    {
        const int wid_x = get_global_id(0);
        const int wid_y = get_global_id(1) << 6;
    
        float16 a, a1, a2, a3;
        float16 b, b1, b2, b3;
        int   b_idx = wid_x << 6;
        int   idx = matrix_a_width * wid_y + b_idx;
        int   idx2 = wid_y * result_width + wid_x;
        b = vload16(0, matrix_b + b_idx);
        b1 = vload16(0, matrix_b + b_idx + 16);
        b2 = vload16(0, matrix_b + b_idx + 32);
        b3 = vload16(0, matrix_b + b_idx + 48);
    
    #pragma unroll 
        for (int i = 0; i < 64; ++i)
        {
            a = vload16(0, matrix_a + idx);
            a1 = vload16(0, matrix_a + idx + 16);
            a2 = vload16(0, matrix_a + idx + 32);
            a3 = vload16(0, matrix_a + idx + 48);
//                matrix_c[idx2 + result_width*i] = dot(a.s0123, b.s0123) + dot(a.s4567, b.s4567) + dot(a.s89ab, b.s89ab) + dot(a.scdef, b.scdef) + dot(a1.s0123, b1.s0123) + dot(a1.s4567, b1.s4567) + dot(a1.s89ab, b1.s89ab) + dot(a1.scdef, b1.scdef) +
//        dot(a2.s0123, b2.s0123) + dot(a2.s4567, b2.s4567) + dot(a2.s89ab, b2.s89ab) + dot(a2.scdef, b2.scdef) + dot(a3.s0123, b3.s0123) + dot(a3.s4567, b3.s4567) + dot(a3.s89ab, b3.s89ab) + dot(a3.scdef, b3.scdef);
            matrix_c[idx2 + result_width*i] = a.s0*b.s0 + a.s1*b.s1 + a.s2*b.s2 + a.s3*b.s3 + a.s4*b.s4 + a.s5*b.s5 + a.s6*b.s6 + a.s7*b.s7 + a.s8*b.s8 + a.s9*b.s9 + a.sA*b.sA + a.sB*b.sB + a.sC*b.sC + a.sD*b.sD + a.sE*b.sE + a.sF*b.sF + 
               a2.s0*b2.s0 + a2.s1*b2.s1 + a2.s2*b2.s2 + a2.s3*b2.s3 + a2.s4*b2.s4 + a2.s5*b2.s5 + a2.s6*b2.s6 + a2.s7*b2.s7 + a2.s8*b2.s8 + a2.s9*b2.s9 + a2.sA*b2.sA + a2.sB*b2.sB + a2.sC*b2.sC + a2.sD*b2.sD + a2.sE*b2.sE + a2.sF*b2.sF + 
               a3.s0*b3.s0 + a3.s1*b3.s1 + a3.s2*b3.s2 + a3.s3*b3.s3 + a3.s4*b3.s4 + a3.s5*b3.s5 + a3.s6*b3.s6 + a3.s7*b3.s7 + a3.s8*b3.s8 + a3.s9*b3.s9 + a3.sA*b3.sA + a3.sB*b3.sB + a3.sC*b3.sC + a3.sD*b3.sD + a3.sE*b3.sE + a3.sF*b3.sF + 
               a1.s0*b1.s0 + a1.s1*b1.s1 + a1.s2*b1.s2 + a1.s3*b1.s3 + a1.s4*b1.s4 + a1.s5*b1.s5 + a1.s6*b1.s6 + a1.s7*b1.s7 + a1.s8*b1.s8 + a1.s9*b1.s9 + a1.sA*b1.sA + a1.sB*b1.sB + a1.sC*b1.sC + a1.sD*b1.sD + a1.sE*b1.sE + a1.sF*b1.sF;
            idx += matrix_a_width;
        }
    
    }
    
   __kernel void matmul_reduce(__global const  float   * restrict matrix_a,
                                   __global        float * restrict matrix_c,
                                                   int    matrix_a_width,
                                                   int    step_count)
    {
        const int wid_y = get_global_id(0);
        const int idx = wid_y * matrix_a_width;
    
        float c = 0.0f;
        float16 a;
    
    #pragma unroll 
        for (int i = 0; i < step_count; ++i)
        {
            a = vload16(0, matrix_a + idx + i*16);
            c += a.s0 + a.s1 + a.s2 + a.s3 + a.s4 + a.s5 + a.s6 + a.s7 + a.s8 + a.s9 + a.sA + a.sB + a.sC + a.sD + a.sE + a.sF;
        }
//unfortunately we have to hardcode the remainder here, when the matrix width is not divisible by stepSize
//otherwise the overhead of calculating this in a separate kernel is a bit too high
        matrix_c[wid_y] = c + matrix_a[idx + 176];
    }
