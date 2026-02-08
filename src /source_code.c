#include <riscv_vector.h>
#include <stdint.h>

void q15_axpy_rvv(const int16_t *a, const int16_t *b, int16_t *y, int n, int16_t alpha) {
    // 'vl' will store the number of elements processed in the current iteration
    size_t vl;

    // Loop until all 'n' elements are processed
    for (; n > 0; n -= vl) {
        
        // 1. Dynamic Strip Mining:
        // Calculate the vector length (vl) for 16-bit elements (e16) 
        // grouped into 8 registers (m8) for maximum throughput.
        vl = __riscv_vsetvl_e16m8(n);

        // 2. Vector Load:
        // Load 'vl' elements from input arrays 'a' and 'b' into vector registers.
        vint16m8_t va = __riscv_vle16_v_i16m8(a, vl);
        vint16m8_t vb = __riscv_vle16_v_i16m8(b, vl);

        // 3. Fixed-Point Multiply (Q15):
        // Multiply 'b' by scalar 'alpha'. 
        // The 'vsmul' intrinsic handles the fixed-point shift and saturation automatically.
        vint16m8_t v_mult_result = __riscv_vsmul_vx_i16m8(vb, alpha, __RISCV_VXRM_RNU, vl);

        // 4. Accumulate with Saturation:
        // Add the multiplication result to 'a'. 
        // 'vsadd' ensures the result saturates to 16-bit limits on overflow.
        vint16m8_t v_final_result = __riscv_vsadd_vv_i16m8(va, v_mult_result, vl);

        // 5. Vector Store:
        // Write the processed chunk back to output memory 'y'.
        __riscv_vse16_v_i16m8(y, v_final_result, vl);

        // 6. Pointer Adjustment:
        // Advance pointers by the number of elements processed.
        a += vl;
        b += vl;
        y += vl;
    }
}
