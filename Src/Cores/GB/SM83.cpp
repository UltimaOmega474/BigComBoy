/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "SM83.hpp"
#include "Bus.hpp"
#include "Constants.hpp"
#include "Core.hpp"
#include <exception>
#include <iostream>

namespace GB
{
    constexpr int16_t NO_DISPLACEMENT = 0;
    constexpr int16_t INCREMENT = 1;
    constexpr int16_t DECREMENT = -1;
    constexpr auto WITH_CARRY = true;
    constexpr auto WITHOUT_CARRY = false;
    constexpr auto DONT_SET_IME = false;
    constexpr auto SET_IME = true;

    SM83::SM83(Core &core, MainBus &bus)
        : core(core), bus(bus), opcodes(gen_optable()), cb_opcodes(gen_cb_optable())
    {
    }

    std::array<SM83::opcode_function, 256> SM83::gen_optable()
    {
        return std::array<SM83::opcode_function, 256>{

            // 0x00 - 0x0F
            &SM83::op_nop<false, 0>,
            &SM83::op_ld_rp_u16<BC>,
            &SM83::op_ld_rp_a<BC, NO_DISPLACEMENT>,
            &SM83::op_inc_rp<BC>,
            &SM83::op_inc_r<B>,
            &SM83::op_dec_r<B>,
            &SM83::op_ld_r_u8<B>,
            &SM83::op_rlca,
            &SM83::op_ld_u16_sp,
            &SM83::op_add_hl_rp<BC>,
            &SM83::op_ld_a_rp<BC, NO_DISPLACEMENT>,
            &SM83::op_dec_rp<BC>,
            &SM83::op_inc_r<C>,
            &SM83::op_dec_r<C>,
            &SM83::op_ld_r_u8<C>,
            &SM83::op_rrca,

            // 0x10 - 0x1F
            &SM83::op_stop,
            &SM83::op_ld_rp_u16<DE>,
            &SM83::op_ld_rp_a<DE, NO_DISPLACEMENT>,
            &SM83::op_inc_rp<DE>,
            &SM83::op_inc_r<D>,
            &SM83::op_dec_r<D>,
            &SM83::op_ld_r_u8<D>,
            &SM83::op_rla,
            &SM83::op_jr_i8,
            &SM83::op_add_hl_rp<DE>,
            &SM83::op_ld_a_rp<DE, NO_DISPLACEMENT>,
            &SM83::op_dec_rp<DE>,
            &SM83::op_inc_r<E>,
            &SM83::op_dec_r<E>,
            &SM83::op_ld_r_u8<E>,
            &SM83::op_rra,

            // 0x20 - 0x2F
            &SM83::op_jr_cc_i8<Z, false>,
            &SM83::op_ld_rp_u16<HL>,
            &SM83::op_ld_rp_a<HL, INCREMENT>,
            &SM83::op_inc_rp<HL>,
            &SM83::op_inc_r<H>,
            &SM83::op_dec_r<H>,
            &SM83::op_ld_r_u8<H>,
            &SM83::op_daa,
            &SM83::op_jr_cc_i8<Z, true>,
            &SM83::op_add_hl_rp<HL>,
            &SM83::op_ld_a_rp<HL, INCREMENT>,
            &SM83::op_dec_rp<HL>,
            &SM83::op_inc_r<L>,
            &SM83::op_dec_r<L>,
            &SM83::op_ld_r_u8<L>,
            &SM83::op_cpl,

            // 0x30 - 0x3F
            &SM83::op_jr_cc_i8<CY, false>,
            &SM83::op_ld_rp_u16<SP>,
            &SM83::op_ld_rp_a<HL, DECREMENT>,
            &SM83::op_inc_rp<SP>,
            &SM83::op_inc_r<HL_ADDR>,
            &SM83::op_dec_r<HL_ADDR>,
            &SM83::op_ld_r_u8<HL_ADDR>,
            &SM83::op_scf,
            &SM83::op_jr_cc_i8<CY, true>,
            &SM83::op_add_hl_rp<SP>,
            &SM83::op_ld_a_rp<HL, DECREMENT>,
            &SM83::op_dec_rp<SP>,
            &SM83::op_inc_r<A>,
            &SM83::op_dec_r<A>,
            &SM83::op_ld_r_u8<A>,
            &SM83::op_ccf,

            // 0x40 - 0x4F
            &SM83::op_ld_r_r<B, B>,
            &SM83::op_ld_r_r<B, C>,
            &SM83::op_ld_r_r<B, D>,
            &SM83::op_ld_r_r<B, E>,
            &SM83::op_ld_r_r<B, H>,
            &SM83::op_ld_r_r<B, L>,
            &SM83::op_ld_r_r<B, HL_ADDR>,
            &SM83::op_ld_r_r<B, A>,
            &SM83::op_ld_r_r<C, B>,
            &SM83::op_ld_r_r<C, C>,
            &SM83::op_ld_r_r<C, D>,
            &SM83::op_ld_r_r<C, E>,
            &SM83::op_ld_r_r<C, H>,
            &SM83::op_ld_r_r<C, L>,
            &SM83::op_ld_r_r<C, HL_ADDR>,
            &SM83::op_ld_r_r<C, A>,

            // 0x50 - 0x5F
            &SM83::op_ld_r_r<D, B>,
            &SM83::op_ld_r_r<D, C>,
            &SM83::op_ld_r_r<D, D>,
            &SM83::op_ld_r_r<D, E>,
            &SM83::op_ld_r_r<D, H>,
            &SM83::op_ld_r_r<D, L>,
            &SM83::op_ld_r_r<D, HL_ADDR>,
            &SM83::op_ld_r_r<D, A>,
            &SM83::op_ld_r_r<E, B>,
            &SM83::op_ld_r_r<E, C>,
            &SM83::op_ld_r_r<E, D>,
            &SM83::op_ld_r_r<E, E>,
            &SM83::op_ld_r_r<E, H>,
            &SM83::op_ld_r_r<E, L>,
            &SM83::op_ld_r_r<E, HL_ADDR>,
            &SM83::op_ld_r_r<E, A>,

            // 0x60 - 0x6F
            &SM83::op_ld_r_r<H, B>,
            &SM83::op_ld_r_r<H, C>,
            &SM83::op_ld_r_r<H, D>,
            &SM83::op_ld_r_r<H, E>,
            &SM83::op_ld_r_r<H, H>,
            &SM83::op_ld_r_r<H, L>,
            &SM83::op_ld_r_r<H, HL_ADDR>,
            &SM83::op_ld_r_r<H, A>,
            &SM83::op_ld_r_r<L, B>,
            &SM83::op_ld_r_r<L, C>,
            &SM83::op_ld_r_r<L, D>,
            &SM83::op_ld_r_r<L, E>,
            &SM83::op_ld_r_r<L, H>,
            &SM83::op_ld_r_r<L, L>,
            &SM83::op_ld_r_r<L, HL_ADDR>,
            &SM83::op_ld_r_r<L, A>,

            // 0x70 - 0x7F
            &SM83::op_ld_r_r<HL_ADDR, B>,
            &SM83::op_ld_r_r<HL_ADDR, C>,
            &SM83::op_ld_r_r<HL_ADDR, D>,
            &SM83::op_ld_r_r<HL_ADDR, E>,
            &SM83::op_ld_r_r<HL_ADDR, H>,
            &SM83::op_ld_r_r<HL_ADDR, L>,
            // HALT
            &SM83::op_ld_r_r<HL_ADDR, HL_ADDR>,
            &SM83::op_ld_r_r<HL_ADDR, A>,
            &SM83::op_ld_r_r<A, B>,
            &SM83::op_ld_r_r<A, C>,
            &SM83::op_ld_r_r<A, D>,
            &SM83::op_ld_r_r<A, E>,
            &SM83::op_ld_r_r<A, H>,
            &SM83::op_ld_r_r<A, L>,
            &SM83::op_ld_r_r<A, HL_ADDR>,
            &SM83::op_ld_r_r<A, A>,

            // 0x80 - 0x8F
            &SM83::op_add_a_r<B, WITHOUT_CARRY>,
            &SM83::op_add_a_r<C, WITHOUT_CARRY>,
            &SM83::op_add_a_r<D, WITHOUT_CARRY>,
            &SM83::op_add_a_r<E, WITHOUT_CARRY>,
            &SM83::op_add_a_r<H, WITHOUT_CARRY>,
            &SM83::op_add_a_r<L, WITHOUT_CARRY>,
            &SM83::op_add_a_r<HL_ADDR, WITHOUT_CARRY>,
            &SM83::op_add_a_r<A, WITHOUT_CARRY>,
            &SM83::op_add_a_r<B, WITH_CARRY>,
            &SM83::op_add_a_r<C, WITH_CARRY>,
            &SM83::op_add_a_r<D, WITH_CARRY>,
            &SM83::op_add_a_r<E, WITH_CARRY>,
            &SM83::op_add_a_r<H, WITH_CARRY>,
            &SM83::op_add_a_r<L, WITH_CARRY>,
            &SM83::op_add_a_r<HL_ADDR, WITH_CARRY>,
            &SM83::op_add_a_r<A, WITH_CARRY>,

            // 0x90 - 0x9F
            &SM83::op_sub_a_r<B, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<C, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<D, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<E, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<H, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<L, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<HL_ADDR, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<A, WITHOUT_CARRY>,
            &SM83::op_sub_a_r<B, WITH_CARRY>,
            &SM83::op_sub_a_r<C, WITH_CARRY>,
            &SM83::op_sub_a_r<D, WITH_CARRY>,
            &SM83::op_sub_a_r<E, WITH_CARRY>,
            &SM83::op_sub_a_r<H, WITH_CARRY>,
            &SM83::op_sub_a_r<L, WITH_CARRY>,
            &SM83::op_sub_a_r<HL_ADDR, WITH_CARRY>,
            &SM83::op_sub_a_r<A, WITH_CARRY>,

            // 0xA0 - 0xAF
            &SM83::op_and_a_r<B>,
            &SM83::op_and_a_r<C>,
            &SM83::op_and_a_r<D>,
            &SM83::op_and_a_r<E>,
            &SM83::op_and_a_r<H>,
            &SM83::op_and_a_r<L>,
            &SM83::op_and_a_r<HL_ADDR>,
            &SM83::op_and_a_r<A>,
            &SM83::op_xor_a_r<B>,
            &SM83::op_xor_a_r<C>,
            &SM83::op_xor_a_r<D>,
            &SM83::op_xor_a_r<E>,
            &SM83::op_xor_a_r<H>,
            &SM83::op_xor_a_r<L>,
            &SM83::op_xor_a_r<HL_ADDR>,
            &SM83::op_xor_a_r<A>,

            // 0xB0 - 0xBF
            &SM83::op_or_a_r<B>,
            &SM83::op_or_a_r<C>,
            &SM83::op_or_a_r<D>,
            &SM83::op_or_a_r<E>,
            &SM83::op_or_a_r<H>,
            &SM83::op_or_a_r<L>,
            &SM83::op_or_a_r<HL_ADDR>,
            &SM83::op_or_a_r<A>,
            &SM83::op_cp_a_r<B>,
            &SM83::op_cp_a_r<C>,
            &SM83::op_cp_a_r<D>,
            &SM83::op_cp_a_r<E>,
            &SM83::op_cp_a_r<H>,
            &SM83::op_cp_a_r<L>,
            &SM83::op_cp_a_r<HL_ADDR>,
            &SM83::op_cp_a_r<A>,

            // 0xC0 - 0xCF
            &SM83::op_ret_cc<Z, false>,
            &SM83::op_pop_rp<BC>,
            &SM83::op_jp_cc_u16<Z, false>,
            &SM83::op_jp_u16,
            &SM83::op_call_cc_u16<Z, false>,
            &SM83::op_push_rp<BC>,
            &SM83::op_add_a_r<U8, WITHOUT_CARRY>,
            &SM83::op_rst_n<0x00>,
            &SM83::op_ret_cc<Z, true>,
            &SM83::op_ret<DONT_SET_IME>,
            &SM83::op_jp_cc_u16<Z, true>,
            &SM83::op_cb,
            &SM83::op_call_cc_u16<Z, true>,
            &SM83::op_call_u16,
            &SM83::op_add_a_r<U8, WITH_CARRY>,
            &SM83::op_rst_n<0x08>,

            // 0xD0 - 0xDF
            &SM83::op_ret_cc<CY, false>,
            &SM83::op_pop_rp<DE>,
            &SM83::op_jp_cc_u16<CY, false>,
            &SM83::op_nop<true, 0xD3>,
            &SM83::op_call_cc_u16<CY, false>,
            &SM83::op_push_rp<DE>,
            &SM83::op_sub_a_r<U8, WITHOUT_CARRY>,
            &SM83::op_rst_n<0x10>,
            &SM83::op_ret_cc<CY, true>,
            &SM83::op_ret<SET_IME>,
            &SM83::op_jp_cc_u16<CY, true>,
            &SM83::op_nop<true, 0xDB>,
            &SM83::op_call_cc_u16<CY, true>,
            &SM83::op_nop<true, 0xDD>,
            &SM83::op_sub_a_r<U8, WITH_CARRY>,
            &SM83::op_rst_n<0x18>,

            // 0xE0 - 0xEF
            &SM83::op_ld_ff00_u8_a,
            &SM83::op_pop_rp<HL>,
            &SM83::op_ld_ff00_c_a,
            &SM83::op_nop<true, 0xE3>,
            &SM83::op_nop<true, 0xE4>,
            &SM83::op_push_rp<HL>,
            &SM83::op_and_a_r<U8>,
            &SM83::op_rst_n<0x20>,
            &SM83::op_add_sp_i8,
            &SM83::op_jp_hl,
            &SM83::op_ld_u16_a,
            &SM83::op_nop<true, 0xEB>,
            &SM83::op_nop<true, 0xEC>,
            &SM83::op_nop<true, 0xED>,
            &SM83::op_xor_a_r<U8>,
            &SM83::op_rst_n<0x28>,

            // 0xF0 - 0xFF
            &SM83::op_ld_a_ff00_u8,
            &SM83::op_pop_rp<AF>,
            &SM83::op_ld_a_ff00_c,
            &SM83::op_di,
            &SM83::op_nop<true, 0xF4>,
            &SM83::op_push_rp<AF>,
            &SM83::op_or_a_r<U8>,
            &SM83::op_rst_n<0x30>,
            &SM83::op_ld_hl_sp_i8,
            &SM83::op_ld_sp_hl,
            &SM83::op_ld_a_u16,
            &SM83::op_ei,
            &SM83::op_nop<true, 0xFC>,
            &SM83::op_nop<true, 0xFD>,
            &SM83::op_cp_a_r<U8>,
            &SM83::op_rst_n<0x38>,
        };
    }

    std::array<SM83::opcode_function, 256> SM83::gen_cb_optable()
    {
        return std::array<SM83::opcode_function, 256>{
            // 0x00 - 0x0F
            &SM83::op_rlc<B>,
            &SM83::op_rlc<C>,
            &SM83::op_rlc<D>,
            &SM83::op_rlc<E>,
            &SM83::op_rlc<H>,
            &SM83::op_rlc<L>,
            &SM83::op_rlc<HL_ADDR>,
            &SM83::op_rlc<A>,

            &SM83::op_rrc<B>,
            &SM83::op_rrc<C>,
            &SM83::op_rrc<D>,
            &SM83::op_rrc<E>,
            &SM83::op_rrc<H>,
            &SM83::op_rrc<L>,
            &SM83::op_rrc<HL_ADDR>,
            &SM83::op_rrc<A>,

            // 0x10 - 0x1F
            &SM83::op_rl<B>,
            &SM83::op_rl<C>,
            &SM83::op_rl<D>,
            &SM83::op_rl<E>,
            &SM83::op_rl<H>,
            &SM83::op_rl<L>,
            &SM83::op_rl<HL_ADDR>,
            &SM83::op_rl<A>,

            &SM83::op_rr<B>,
            &SM83::op_rr<C>,
            &SM83::op_rr<D>,
            &SM83::op_rr<E>,
            &SM83::op_rr<H>,
            &SM83::op_rr<L>,
            &SM83::op_rr<HL_ADDR>,
            &SM83::op_rr<A>,

            // 0x20 - 0x2F
            &SM83::op_sla<B>,
            &SM83::op_sla<C>,
            &SM83::op_sla<D>,
            &SM83::op_sla<E>,
            &SM83::op_sla<H>,
            &SM83::op_sla<L>,
            &SM83::op_sla<HL_ADDR>,
            &SM83::op_sla<A>,

            &SM83::op_sra<B>,
            &SM83::op_sra<C>,
            &SM83::op_sra<D>,
            &SM83::op_sra<E>,
            &SM83::op_sra<H>,
            &SM83::op_sra<L>,
            &SM83::op_sra<HL_ADDR>,
            &SM83::op_sra<A>,

            // 0x30 - 0x3F
            &SM83::op_swap<B>,
            &SM83::op_swap<C>,
            &SM83::op_swap<D>,
            &SM83::op_swap<E>,
            &SM83::op_swap<H>,
            &SM83::op_swap<L>,
            &SM83::op_swap<HL_ADDR>,
            &SM83::op_swap<A>,

            &SM83::op_srl<B>,
            &SM83::op_srl<C>,
            &SM83::op_srl<D>,
            &SM83::op_srl<E>,
            &SM83::op_srl<H>,
            &SM83::op_srl<L>,
            &SM83::op_srl<HL_ADDR>,
            &SM83::op_srl<A>,

            // 0x40 - 0x4F
            &SM83::op_bit<0, B>,
            &SM83::op_bit<0, C>,
            &SM83::op_bit<0, D>,
            &SM83::op_bit<0, E>,
            &SM83::op_bit<0, H>,
            &SM83::op_bit<0, L>,
            &SM83::op_bit<0, HL_ADDR>,
            &SM83::op_bit<0, A>,

            &SM83::op_bit<1, B>,
            &SM83::op_bit<1, C>,
            &SM83::op_bit<1, D>,
            &SM83::op_bit<1, E>,
            &SM83::op_bit<1, H>,
            &SM83::op_bit<1, L>,
            &SM83::op_bit<1, HL_ADDR>,
            &SM83::op_bit<1, A>,

            // 0x50 - 0x5F
            &SM83::op_bit<2, B>,
            &SM83::op_bit<2, C>,
            &SM83::op_bit<2, D>,
            &SM83::op_bit<2, E>,
            &SM83::op_bit<2, H>,
            &SM83::op_bit<2, L>,
            &SM83::op_bit<2, HL_ADDR>,
            &SM83::op_bit<2, A>,

            &SM83::op_bit<3, B>,
            &SM83::op_bit<3, C>,
            &SM83::op_bit<3, D>,
            &SM83::op_bit<3, E>,
            &SM83::op_bit<3, H>,
            &SM83::op_bit<3, L>,
            &SM83::op_bit<3, HL_ADDR>,
            &SM83::op_bit<3, A>,

            // 0x60 - 0x6F
            &SM83::op_bit<4, B>,
            &SM83::op_bit<4, C>,
            &SM83::op_bit<4, D>,
            &SM83::op_bit<4, E>,
            &SM83::op_bit<4, H>,
            &SM83::op_bit<4, L>,
            &SM83::op_bit<4, HL_ADDR>,
            &SM83::op_bit<4, A>,

            &SM83::op_bit<5, B>,
            &SM83::op_bit<5, C>,
            &SM83::op_bit<5, D>,
            &SM83::op_bit<5, E>,
            &SM83::op_bit<5, H>,
            &SM83::op_bit<5, L>,
            &SM83::op_bit<5, HL_ADDR>,
            &SM83::op_bit<5, A>,

            // 0x70 - 0x7F
            &SM83::op_bit<6, B>,
            &SM83::op_bit<6, C>,
            &SM83::op_bit<6, D>,
            &SM83::op_bit<6, E>,
            &SM83::op_bit<6, H>,
            &SM83::op_bit<6, L>,
            &SM83::op_bit<6, HL_ADDR>,
            &SM83::op_bit<6, A>,

            &SM83::op_bit<7, B>,
            &SM83::op_bit<7, C>,
            &SM83::op_bit<7, D>,
            &SM83::op_bit<7, E>,
            &SM83::op_bit<7, H>,
            &SM83::op_bit<7, L>,
            &SM83::op_bit<7, HL_ADDR>,
            &SM83::op_bit<7, A>,

            // 0x80 - 0x8F
            &SM83::op_res<0, B>,
            &SM83::op_res<0, C>,
            &SM83::op_res<0, D>,
            &SM83::op_res<0, E>,
            &SM83::op_res<0, H>,
            &SM83::op_res<0, L>,
            &SM83::op_res<0, HL_ADDR>,
            &SM83::op_res<0, A>,

            &SM83::op_res<1, B>,
            &SM83::op_res<1, C>,
            &SM83::op_res<1, D>,
            &SM83::op_res<1, E>,
            &SM83::op_res<1, H>,
            &SM83::op_res<1, L>,
            &SM83::op_res<1, HL_ADDR>,
            &SM83::op_res<1, A>,

            // 0x90 - 0x9F
            &SM83::op_res<2, B>,
            &SM83::op_res<2, C>,
            &SM83::op_res<2, D>,
            &SM83::op_res<2, E>,
            &SM83::op_res<2, H>,
            &SM83::op_res<2, L>,
            &SM83::op_res<2, HL_ADDR>,
            &SM83::op_res<2, A>,

            &SM83::op_res<3, B>,
            &SM83::op_res<3, C>,
            &SM83::op_res<3, D>,
            &SM83::op_res<3, E>,
            &SM83::op_res<3, H>,
            &SM83::op_res<3, L>,
            &SM83::op_res<3, HL_ADDR>,
            &SM83::op_res<3, A>,

            // 0xA0 - 0xAF
            &SM83::op_res<4, B>,
            &SM83::op_res<4, C>,
            &SM83::op_res<4, D>,
            &SM83::op_res<4, E>,
            &SM83::op_res<4, H>,
            &SM83::op_res<4, L>,
            &SM83::op_res<4, HL_ADDR>,
            &SM83::op_res<4, A>,

            &SM83::op_res<5, B>,
            &SM83::op_res<5, C>,
            &SM83::op_res<5, D>,
            &SM83::op_res<5, E>,
            &SM83::op_res<5, H>,
            &SM83::op_res<5, L>,
            &SM83::op_res<5, HL_ADDR>,
            &SM83::op_res<5, A>,

            // 0xB0 - 0xBF
            &SM83::op_res<6, B>,
            &SM83::op_res<6, C>,
            &SM83::op_res<6, D>,
            &SM83::op_res<6, E>,
            &SM83::op_res<6, H>,
            &SM83::op_res<6, L>,
            &SM83::op_res<6, HL_ADDR>,
            &SM83::op_res<6, A>,

            &SM83::op_res<7, B>,
            &SM83::op_res<7, C>,
            &SM83::op_res<7, D>,
            &SM83::op_res<7, E>,
            &SM83::op_res<7, H>,
            &SM83::op_res<7, L>,
            &SM83::op_res<7, HL_ADDR>,
            &SM83::op_res<7, A>,

            // 0xC0 - 0xCF
            &SM83::op_set<0, B>,
            &SM83::op_set<0, C>,
            &SM83::op_set<0, D>,
            &SM83::op_set<0, E>,
            &SM83::op_set<0, H>,
            &SM83::op_set<0, L>,
            &SM83::op_set<0, HL_ADDR>,
            &SM83::op_set<0, A>,

            &SM83::op_set<1, B>,
            &SM83::op_set<1, C>,
            &SM83::op_set<1, D>,
            &SM83::op_set<1, E>,
            &SM83::op_set<1, H>,
            &SM83::op_set<1, L>,
            &SM83::op_set<1, HL_ADDR>,
            &SM83::op_set<1, A>,

            // 0xD0 - 0xDF
            &SM83::op_set<2, B>,
            &SM83::op_set<2, C>,
            &SM83::op_set<2, D>,
            &SM83::op_set<2, E>,
            &SM83::op_set<2, H>,
            &SM83::op_set<2, L>,
            &SM83::op_set<2, HL_ADDR>,
            &SM83::op_set<2, A>,

            &SM83::op_set<3, B>,
            &SM83::op_set<3, C>,
            &SM83::op_set<3, D>,
            &SM83::op_set<3, E>,
            &SM83::op_set<3, H>,
            &SM83::op_set<3, L>,
            &SM83::op_set<3, HL_ADDR>,
            &SM83::op_set<3, A>,

            // 0xE0 - 0xEF
            &SM83::op_set<4, B>,
            &SM83::op_set<4, C>,
            &SM83::op_set<4, D>,
            &SM83::op_set<4, E>,
            &SM83::op_set<4, H>,
            &SM83::op_set<4, L>,
            &SM83::op_set<4, HL_ADDR>,
            &SM83::op_set<4, A>,

            &SM83::op_set<5, B>,
            &SM83::op_set<5, C>,
            &SM83::op_set<5, D>,
            &SM83::op_set<5, E>,
            &SM83::op_set<5, H>,
            &SM83::op_set<5, L>,
            &SM83::op_set<5, HL_ADDR>,
            &SM83::op_set<5, A>,

            // 0xF0 - 0xFF
            &SM83::op_set<6, B>,
            &SM83::op_set<6, C>,
            &SM83::op_set<6, D>,
            &SM83::op_set<6, E>,
            &SM83::op_set<6, H>,
            &SM83::op_set<6, L>,
            &SM83::op_set<6, HL_ADDR>,
            &SM83::op_set<6, A>,

            &SM83::op_set<7, B>,
            &SM83::op_set<7, C>,
            &SM83::op_set<7, D>,
            &SM83::op_set<7, E>,
            &SM83::op_set<7, H>,
            &SM83::op_set<7, L>,
            &SM83::op_set<7, HL_ADDR>,
            &SM83::op_set<7, A>,
        };
    }

    void SM83::reset(uint16_t new_pc)
    {
        master_interrupt_enable = false;
        double_speed = false;
        halted = false;
        stopped = false;
        ei_delay = false;
        interrupt_enable = 0;
        interrupt_flag = 0;

        if (bus.is_compatibility_mode())
        {
            set_flags(N, false);
            set_flags(Z, true);
            set_flags(HC | CY, true);

            registers[A] = 0x01;
            registers[B] = 0x00;
            registers[C] = 0x13;
            registers[D] = 0x00;
            registers[E] = 0xD8;
            registers[H] = 0x01;
            registers[L] = 0x4D;

            pc = new_pc;
            sp = 0xFFFE;
        }
        else
        {
            set_flags(N | HC | CY, false);
            set_flags(Z, true);

            registers[A] = 0x11;
            registers[B] = 0x00;
            registers[C] = 0x00;
            registers[D] = 0xFF;
            registers[E] = 0x56;
            registers[H] = 0x00;
            registers[L] = 0x0D;

            pc = new_pc;
            sp = 0xFFFE;
        }
    }

    void SM83::step()
    {
        service_interrupts();
        if (ei_delay)
        {
            master_interrupt_enable = true;
            ei_delay = false;
        }

        auto opcode = read(pc);

        if (halted)
            return;

        (this->*opcodes.at(opcode))();
    }

    void SM83::service_interrupts()
    {
        uint8_t interrupt_pending = interrupt_flag & interrupt_enable;

        if (interrupt_pending)
        {
            /*
                    The CPU wakes up from HALT if any interrupt has been signaled/pending.
                    This happens regardless of IME which only controls whether or not the pending
               interrupts will be serviced.
            */

            if (halted)
                halted = false;

            if (!master_interrupt_enable)
                return;

            master_interrupt_enable = false;
            if (interrupt_pending & 0x01)
            {
                core.tick_subcomponents(8);
                push_sp(pc);
                core.tick_subcomponents(4);
                pc = 0x40;

                interrupt_flag &= ~INT_VBLANK_BIT;
            }
            else if (interrupt_pending & INT_LCD_STAT_BIT)
            {
                core.tick_subcomponents(8);
                push_sp(pc);
                core.tick_subcomponents(4);
                pc = 0x48;

                interrupt_flag &= ~INT_LCD_STAT_BIT;
            }
            else if (interrupt_pending & INT_TIMER_BIT)
            {
                core.tick_subcomponents(8);
                push_sp(pc);
                core.tick_subcomponents(4);
                pc = 0x50;

                interrupt_flag &= ~INT_TIMER_BIT;
            }
            else if (interrupt_pending & INT_SERIAL_PORT_BIT)
            {
                core.tick_subcomponents(8);
                push_sp(pc);
                core.tick_subcomponents(4);
                pc = 0x58;

                interrupt_flag &= ~INT_SERIAL_PORT_BIT;
            }
            else if (interrupt_pending & INT_JOYPAD_BIT)
            {
                core.tick_subcomponents(8);
                push_sp(pc);
                core.tick_subcomponents(4);
                pc = 0x60;

                interrupt_flag &= ~INT_JOYPAD_BIT;
            }
        }
    }

    uint8_t SM83::read(uint16_t address)
    {
        core.tick_subcomponents(4);
        return bus.read(address);
    }

    uint16_t SM83::read_uint16(uint16_t address)
    {
        uint8_t low = read(address);
        uint8_t hi = read(address + 1);

        return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(low);
    }

    void SM83::write(uint16_t address, uint8_t value)
    {
        core.tick_subcomponents(4);
        bus.write(address, value);
    }

    void SM83::write_uint16(uint16_t address, uint16_t value)
    {
        write(address, value & 0x00FF);
        write(address + 1, (value & 0xFF00) >> 8);
    }

    void SM83::push_sp(uint16_t temp)
    {
        write(--sp, (temp & 0xFF00) >> 8);
        write(--sp, (temp & 0xFF));
    }

    uint16_t SM83::pop_sp()
    {
        auto low = read(sp++);
        auto hi = read(sp++);
        return (hi << 8) | low;
    }

    void SM83::set_flags(uint8_t flagBits, bool set)
    {
        uint8_t *f = &registers[Register::F];

        if (set)
        {
            *f |= flagBits;
        }
        else
        {
            *f &= ~(flagBits);
        }
    }

    bool SM83::get_flag(CPUFlags flagBit) const { return registers[Register::F] & flagBit; }

    uint16_t SM83::get_rp(RegisterPair index) const
    {
        uint16_t hi;
        uint16_t low;
        switch (index)
        {

        case RegisterPair::BC:
        {
            hi = (uint16_t)registers[B];
            low = (uint16_t)registers[C];
            break;
        }

        case RegisterPair::DE:
        {
            hi = (uint16_t)registers[D];
            low = (uint16_t)registers[E];
            break;
        }

        case RegisterPair::HL:
        {
            hi = (uint16_t)registers[H];
            low = (uint16_t)registers[L];
            break;
        }

        case RegisterPair::SP:
        {
            return sp;
        }

        case RegisterPair::AF:
        {
            hi = (uint16_t)registers[A];
            low = (uint16_t)registers[F];
            break;
        }
        }

        return (hi << 8) | (low);
    }

    void SM83::set_rp(RegisterPair index, uint16_t temp)
    {
        switch (index)
        {

        case RegisterPair::BC:
        {
            registers[B] = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            registers[C] = static_cast<uint8_t>(temp & 0x00FF);
            break;
        }

        case RegisterPair::DE:
        {
            registers[D] = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            registers[E] = static_cast<uint8_t>(temp & 0x00FF);
            break;
        }

        case RegisterPair::HL:
        {
            registers[H] = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            registers[L] = static_cast<uint8_t>(temp & 0x00FF);

            break;
        }

        case RegisterPair::SP:
        {
            sp = temp;
            break;
        }
        case RegisterPair::AF:
        {
            registers[A] = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            registers[F] = static_cast<uint8_t>(temp & 0x00FF);
            set_flags(B0 | B1 | B2 | B3, false);
            break;
        }
        }
    }

    void SM83::op_ld_u16_sp()
    {
        auto addr = read_uint16(pc + 1);

        write_uint16(addr, sp);
        pc += 3;
    }

    void SM83::op_stop()
    {
        if (core.bus.is_compatibility_mode())
        {
            stopped = true;
            return;
        }

        if (bus.KEY1 & 0x1)
        {
            double_speed = !double_speed;
            bus.KEY1 = double_speed << 7;
        }

        pc++;
    }

    void SM83::op_jr_i8()
    {
        int8_t off = static_cast<int8_t>(read(pc + 1));

        pc += 2;
        pc += off;
        core.tick_subcomponents(4);
    }

    void SM83::op_rlca()
    {
        uint16_t temp = static_cast<uint16_t>(registers[A]);
        auto bit7 = temp & 0x80 ? 1 : 0;

        set_flags(CY, bit7);
        set_flags(Z | N | HC, false);
        temp = (temp << 1) | bit7;

        registers[A] = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_rrca()
    {
        uint16_t temp = static_cast<uint16_t>(registers[A]);
        uint8_t bit0 = (temp & 1) ? 0x80 : 0;

        set_flags(CY, (temp & 1));
        set_flags(Z | N | HC, false);
        temp = (temp >> 1) | bit0;

        registers[A] = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_rla()
    {
        uint16_t temp = static_cast<uint16_t>(registers[A]);
        uint16_t cy = get_flag(CY);

        set_flags(CY, (temp & 0x80));
        set_flags(Z | N | HC, false);
        temp = (temp << 1) | cy;

        registers[A] = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_rra()
    {
        uint16_t temp = static_cast<uint16_t>(registers[A]);
        uint16_t cy = get_flag(CY);

        cy <<= 7;
        set_flags(CY, (temp & 1));
        set_flags(Z | N | HC, false);
        temp = (temp >> 1) | cy;

        registers[A] = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_daa()
    {
        // Implementation adapted from: https://ehaskins.com/2018-01-30%20Z80%20DAA/
        uint8_t correct = 0;
        uint16_t temp = static_cast<uint16_t>(registers[A]);
        bool cy = false;
        if (get_flag(HC) || (!get_flag(N) && (temp & 0xF) > 9))
        {
            correct |= 6;
        }

        if (get_flag(CY) || (!get_flag(N) && temp > 0x99))
        {
            correct |= 0x60;
            cy = true;
        }

        temp += get_flag(N) ? -correct : correct;
        temp &= 0xFF;

        set_flags(Z, temp == 0);
        set_flags(HC, false);
        set_flags(CY, cy);

        registers[A] = static_cast<uint8_t>(temp);

        ++pc;
    }

    void SM83::op_cpl()
    {
        registers[A] = ~registers[A];
        set_flags(N | HC, true);

        ++pc;
    }

    void SM83::op_scf()
    {
        set_flags(N | HC, false);
        set_flags(CY, true);

        ++pc;
    }

    void SM83::op_ccf()
    {
        set_flags(N | HC, false);
        set_flags(CY, !get_flag(CY));

        ++pc;
    }

    void SM83::op_jp_u16()
    {
        pc = read_uint16(pc + 1);
        core.tick_subcomponents(4);
    }

    void SM83::op_call_u16()
    {
        auto saved_pc = pc + 3;
        auto addr = read_uint16(pc + 1);
        core.tick_subcomponents(4);
        push_sp(saved_pc);
        pc = addr;
    }

    void SM83::op_cb()
    {
        // todo: implement the 0xCB table.
        // gb.tickSubcomponents(8);
        auto opcode = read(pc + 1);

        (this->*cb_opcodes.at(opcode))();
        pc += 2;
    }

    void SM83::op_ld_ff00_u8_a()
    {
        auto off = read(pc + 1);
        write(0xFF00 + off, registers[A]);

        pc += 2;
    }

    void SM83::op_ld_ff00_c_a()
    {
        write(0xFF00 + registers[C], registers[A]);

        ++pc;
    }

    void SM83::op_add_sp_i8()
    {
        int16_t off = static_cast<int8_t>(read(pc + 1));
        uint16_t sp32 = sp;
        uint16_t res32 = (sp32 + off);

        // internal operation?
        core.tick_subcomponents(4);
        set_rp(SP, res32 & 0xFFFF);
        // sp update is visible
        core.tick_subcomponents(4);
        set_flags(HC, ((sp32 & 0xF) + (off & 0xF)) > 0xF);
        set_flags(CY, ((sp32 & 0xFF) + (off & 0xFF)) > 0xFF);
        set_flags(Z | N, false);

        pc += 2;
    }

    void SM83::op_jp_hl() { pc = get_rp(HL); }

    void SM83::op_ld_u16_a()
    {
        auto addr = read_uint16(pc + 1);
        write(addr, registers[A]);
        pc += 3;
    }

    void SM83::op_ld_a_ff00_u8()
    {
        uint16_t off = read(pc + 1);

        registers[A] = read(0xFF00 + off);

        pc += 2;
    }

    void SM83::op_ld_a_ff00_c()
    {
        registers[A] = read(0xFF00 + registers[C]);

        ++pc;
    }

    void SM83::op_di()
    {
        master_interrupt_enable = false;
        ei_delay = false;
        ++pc;
    }

    void SM83::op_ei()
    {
        ei_delay = true;

        ++pc;
    }

    void SM83::op_ld_hl_sp_i8()
    {
        int16_t off = static_cast<int8_t>(read(pc + 1));
        uint16_t sp32 = sp;
        uint16_t res32 = (sp32 + off);
        set_rp(HL, static_cast<uint16_t>(res32 & 0xFFFF));
        set_flags(HC, ((sp32 & 0xF) + (off & 0xF)) > 0xF);
        set_flags(CY, ((sp32 & 0xFF) + (off & 0xFF)) > 0xFF);
        set_flags(Z | N, false);
        core.tick_subcomponents(4);
        pc += 2;
    }

    void SM83::op_ld_sp_hl()
    {
        sp = get_rp(HL);
        core.tick_subcomponents(4);
        ++pc;
    }

    void SM83::op_ld_a_u16()
    {
        auto addr = read_uint16(pc + 1);
        registers[A] = read(addr);

        pc += 3;
    }

    template <bool is_illegal_op, uint8_t illegal_op> void SM83::op_nop()
    {
        if constexpr (is_illegal_op)
        {
            std::cerr << "Illegal Opcode triggered: " << std::hex << illegal_op << std::endl;
            std::terminate();
        }

        ++pc;
    }

    template <RegisterPair rp> inline void SM83::op_ld_rp_u16()
    {
        uint16_t combine = read_uint16(pc + 1);

        set_rp(rp, combine);
        pc += 3;
    }

    template <RegisterPair rp> inline void SM83::op_inc_rp()
    {
        uint8_t *high = nullptr;
        uint8_t *low = nullptr;

        switch (rp)
        {
        case RegisterPair::BC:
        {
            high = &registers[B];
            low = &registers[C];
            break;
        }
        case RegisterPair::DE:
        {
            high = &registers[D];
            low = &registers[E];
            break;
        }
        case RegisterPair::HL:
        {
            high = &registers[H];
            low = &registers[L];
            break;
        }
        case RegisterPair::SP:
        {
            sp++;
            core.tick_subcomponents(4);
            ++pc;
            return;
        }
        case RegisterPair::AF:
        {
            high = &registers[A];
            low = &registers[F];
            break;
        }
        }

        uint16_t res = static_cast<uint16_t>(*high) << 8 | static_cast<uint16_t>(*low);

        ++res;
        *high = static_cast<uint8_t>((res & 0xFF00) >> 8);
        *low = static_cast<uint8_t>(res & 0xFF);

        core.tick_subcomponents(4);
        ++pc;
    }

    template <RegisterPair rp> void SM83::op_dec_rp()
    {
        uint8_t *high = nullptr;
        uint8_t *low = nullptr;

        switch (rp)
        {
        case RegisterPair::BC:
        {
            high = &registers[B];
            low = &registers[C];
            break;
        }
        case RegisterPair::DE:
        {
            high = &registers[D];
            low = &registers[E];
            break;
        }
        case RegisterPair::HL:
        {
            high = &registers[H];
            low = &registers[L];
            break;
        }
        case RegisterPair::SP:
        {
            sp--;
            core.tick_subcomponents(4);
            ++pc;
            return;
        }
        case RegisterPair::AF:
        {
            high = &registers[A];
            low = &registers[F];
            break;
        }
        }

        uint16_t res = static_cast<uint16_t>(*high) << 8 | static_cast<uint16_t>(*low);

        --res;
        *high = static_cast<uint8_t>((res & 0xFF00) >> 8);
        *low = static_cast<uint8_t>(res & 0xFF);

        core.tick_subcomponents(4);
        ++pc;
    }

    template <RegisterPair rp, int16_t displacement> inline void SM83::op_ld_rp_a()
    {
        auto addr = get_rp(rp);

        write(addr, registers[A]);

        if constexpr (displacement != 0)
        {
            set_rp(rp, addr + displacement);
        }

        ++pc;
    }

    template <CPUFlags cc, bool boolean_ver> inline void SM83::op_jr_cc_i8()
    {
        if (get_flag(cc) == boolean_ver)
        {
            int8_t off = static_cast<int8_t>(read(pc + 1));

            pc += 2;
            pc += off;
            core.tick_subcomponents(4);
            return;
        }
        core.tick_subcomponents(4);
        pc += 2;
    }

    template <RegisterPair rp> inline void SM83::op_add_hl_rp()
    {
        uint32_t left = get_rp(HL);
        uint32_t right = get_rp(rp);
        uint32_t res32 = left + right;

        set_flags(N, false);
        set_flags(HC, ((left & 0xFFF) + (right & 0xFFF)) > 0xFFF);
        set_flags(CY, res32 > 0xFFFF);
        set_rp(HL, static_cast<uint16_t>(res32 & 0xFFFF));

        this->core.tick_subcomponents(4);
        ++pc;
    }

    template <Register r> void SM83::op_inc_r()
    {
        uint16_t left;
        uint16_t right = 1;

        if constexpr (r == HL_ADDR)
        {
            left = read(get_rp(HL));
        }
        else
        {
            left = registers[r];
        }

        auto result = left + right;
        auto masked_result = result & 0xFF;

        set_flags(N, false);
        set_flags(Z, masked_result == 0);
        set_flags(HC, ((left & 0xF) + (right & 0xF)) > 0xF);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), static_cast<uint8_t>(masked_result));
        }
        else
        {
            registers[r] = static_cast<uint8_t>(masked_result);
        }

        ++pc;
    }

    template <Register r> void SM83::op_dec_r()
    {
        int16_t left;
        int16_t right = 1;

        if constexpr (r == HL_ADDR)
        {
            left = read(get_rp(HL));
        }
        else
        {
            left = registers[r];
        }

        auto result = left - right;
        auto masked_result = result & 0xFF;

        set_flags(HC, ((left & 0xF) - (right & 0xF)) < 0);
        set_flags(Z, masked_result == 0);
        set_flags(N, true); // only set if subtraction

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), static_cast<uint8_t>(masked_result));
        }
        else
        {
            registers[r] = static_cast<uint8_t>(masked_result);
        }
        ++pc;
    }

    template <Register r> void SM83::op_ld_r_u8()
    {
        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), read(pc + 1));
        }
        else
        {
            registers[r] = read(pc + 1);
        }

        pc += 2;
    }

    template <RegisterPair rp, int16_t displacement> void SM83::op_ld_a_rp()
    {
        auto addr = get_rp(rp);

        registers[A] = read(addr);

        if constexpr (displacement != 0)
        {
            set_rp(rp, addr + displacement);
        }

        ++pc;
    }

    template <Register r, Register r2> void SM83::op_ld_r_r()
    {
        // r = destination
        // r2 = source
        if constexpr (r == HL_ADDR && r2 != HL_ADDR)
        {
            write(get_rp(HL), registers[r2]);
        }
        else if constexpr (r != HL_ADDR && r2 == HL_ADDR)
        {
            registers[r] = read(get_rp(HL));
        }
        else if constexpr (r == HL_ADDR && r2 == HL_ADDR)
        {
            halted = true;
        }
        else if constexpr (r != HL_ADDR && r2 != HL_ADDR)
        {
            registers[r] = registers[r2];
        }
        ++pc;
    }

    template <Register r, bool with_carry> void SM83::op_add_a_r()
    {
        uint16_t left = static_cast<uint16_t>(registers[A]);
        uint16_t right;

        if constexpr (r == HL_ADDR)
        {
            right = static_cast<uint16_t>(read(get_rp(HL)));
        }
        else if constexpr (r == U8)
        {
            right = static_cast<uint16_t>(read(pc + 1));
            ++pc;
        }
        else
        {
            right = static_cast<uint16_t>(registers[r]);
        }

        uint16_t cy = with_carry ? get_flag(CY) : 0;

        uint16_t result = left + right + cy;
        uint8_t masked_result = result & 0xFF;

        set_flags(HC, ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF);
        set_flags(CY, result > 0xFF);
        set_flags(Z, masked_result == 0);
        set_flags(N, false);

        registers[A] = masked_result;
        ++pc;
    }

    template <Register r, bool with_carry> void SM83::op_sub_a_r()
    {
        int16_t left = static_cast<int16_t>(registers[A]);
        int16_t right;

        if constexpr (r == HL_ADDR)
        {
            right = static_cast<int16_t>(read(get_rp(HL)));
        }
        else if constexpr (r == U8)
        {
            right = static_cast<int16_t>(read(pc + 1));
            ++pc;
        }
        else
        {
            right = static_cast<int16_t>(registers[r]);
        }

        int16_t cy = with_carry ? get_flag(CY) : 0;

        int16_t result = left - right - cy;
        uint8_t masked_result = result & 0xFF;

        set_flags(HC, ((left & 0xF) - (right & 0xF) - (cy & 0xF)) < 0);
        set_flags(CY, result < 0);
        set_flags(Z, masked_result == 0);
        set_flags(N, true);

        registers[A] = masked_result;

        ++pc;
    }

    template <Register r> void SM83::op_and_a_r()
    {
        uint8_t right;
        if constexpr (r == HL_ADDR)
        {
            right = read(get_rp(HL));
        }
        else if constexpr (r == U8)
        {
            right = read(pc + 1);
            ++pc;
        }
        else
        {
            right = registers[r];
        }

        uint8_t result = registers[A] & right;
        set_flags(Z, result == 0);
        set_flags(N, false);
        set_flags(HC, true);
        set_flags(CY, false);
        registers[A] = result;

        ++pc;
    }

    template <Register r> void SM83::op_xor_a_r()
    {
        uint8_t right;
        if constexpr (r == HL_ADDR)
        {
            right = read(get_rp(HL));
        }
        else if constexpr (r == U8)
        {
            right = read(pc + 1);
            ++pc;
        }
        else
        {
            right = registers[r];
        }

        uint8_t result = registers[A] ^ right;
        set_flags(Z, result == 0);
        set_flags(N, false);
        set_flags(HC, false);
        set_flags(CY, false);
        registers[A] = result;

        ++pc;
    }

    template <Register r> void SM83::op_or_a_r()
    {
        uint8_t right;
        if constexpr (r == HL_ADDR)
        {
            right = read(get_rp(HL));
        }
        else if constexpr (r == U8)
        {
            right = read(pc + 1);
            ++pc;
        }
        else
        {
            right = registers[r];
        }

        uint8_t result = registers[A] | right;
        set_flags(Z, result == 0);
        set_flags(N, false);
        set_flags(HC, false);
        set_flags(CY, false);
        registers[A] = result;

        ++pc;
    }

    template <Register r> void SM83::op_cp_a_r()
    {
        uint8_t right;
        if constexpr (r == HL_ADDR)
        {
            right = read(get_rp(HL));
        }
        else if constexpr (r == U8)
        {
            right = read(pc + 1);
            ++pc;
        }
        else
        {
            right = registers[r];
        }

        int16_t result = registers[A] - right;
        uint8_t masked_result = result & 0xFF;

        set_flags(HC, ((registers[A] & 0xF) - (right & 0xF)) < 0);
        set_flags(CY, result < 0);
        set_flags(Z, masked_result == 0);
        set_flags(N, true);

        // A is not modified, same as subtract without carry

        ++pc;
    }

    template <bool enable_interrupts> void SM83::op_ret()
    {
        if constexpr (enable_interrupts)
            master_interrupt_enable = 1;

        pc = pop_sp();
        core.tick_subcomponents(4);
    }

    template <CPUFlags cc, bool boolean_ver> void SM83::op_ret_cc()
    {
        core.tick_subcomponents(4);
        if (get_flag(cc) == boolean_ver)
        {
            pc = pop_sp();
            core.tick_subcomponents(4);
            return;
        }

        ++pc;
    }

    template <CPUFlags cc, bool boolean_ver> void SM83::op_jp_cc_u16()
    {
        if (get_flag(cc) == boolean_ver)
        {
            pc = read_uint16(pc + 1);
            core.tick_subcomponents(4);
            return;
        }

        core.tick_subcomponents(8);

        pc += 3;
    }

    template <CPUFlags cc, bool boolean_ver> void SM83::op_call_cc_u16()
    {
        if (get_flag(cc) == boolean_ver)
        {
            auto saved_pc = pc + 3;
            auto addr = read_uint16(pc + 1);
            core.tick_subcomponents(4);
            push_sp(saved_pc);
            pc = addr;
            return;
        }

        core.tick_subcomponents(8);

        pc += 3;
    }

    template <RegisterPair rp> void SM83::op_pop_rp()
    {
        auto temp = pop_sp();
        set_rp(rp, temp);
        ++pc;
    }

    template <RegisterPair rp> void SM83::op_push_rp()
    {
        core.tick_subcomponents(4);

        push_sp(get_rp(rp));
        ++pc;
    }

    template <uint16_t page> void SM83::op_rst_n()
    {
        core.tick_subcomponents(4);
        push_sp(pc + 1);

        pc = page;
    }

    template <Register r> void SM83::op_rlc()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        uint8_t bit7 = (temp & 0x80) ? 1 : 0;

        set_flags(CY, bit7);
        set_flags(Z, temp == 0);
        set_flags(N | HC, false);
        temp = temp << 1;
        temp |= bit7;

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <Register r> void SM83::op_rrc()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        uint8_t bit0 = (temp & 0x01) ? 0x80 : 0;
        set_flags(CY, bit0);
        set_flags(Z, temp == 0);
        set_flags(N | HC, false);
        temp = temp >> 1;
        temp |= bit0;

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <Register r> void SM83::op_rl()
    {
        uint16_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        uint16_t cy = get_flag(CY);
        set_flags(CY, (temp & 0x80));

        temp = temp << 1;
        temp = temp | cy;
        uint8_t t8 = static_cast<uint8_t>(temp & 0xFF);
        set_flags(Z, t8 == 0);
        set_flags(N | HC, false);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), static_cast<uint8_t>(temp));
        }
        else
        {
            registers[r] = t8;
        }
    }

    template <Register r> void SM83::op_rr()
    {
        uint16_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        uint16_t cy = get_flag(CY);
        cy = cy << 7;
        set_flags(CY, (temp & 0x01));
        temp = temp >> 1;
        temp = temp | cy;
        uint8_t t8 = temp & 0xFF;
        set_flags(Z, t8 == 0);
        set_flags(N | HC, false);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), static_cast<uint8_t>(temp));
        }
        else
        {
            registers[r] = t8;
        }
    }

    template <Register r> void SM83::op_sla()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        set_flags(CY, (temp & 0x80));
        temp = temp << 1;
        set_flags(Z, temp == 0);
        set_flags(N | HC, false);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <Register r> void SM83::op_sra()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        uint8_t bit7 = (temp & 0x80);
        set_flags(CY, (temp & 0x01));
        temp = temp >> 1;
        temp |= bit7; // bit7 is left unchanged
        set_flags(Z, temp == 0);
        set_flags(N | HC, false);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <Register r> void SM83::op_swap()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        uint8_t hi = (temp & 0xF0) >> 4;
        uint8_t low = (temp & 0x0F) << 4;
        temp = (low) | hi;

        set_flags(Z, temp == 0);
        set_flags(N | HC | CY, false);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <Register r> void SM83::op_srl()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        set_flags(CY, (temp & 0x01));
        temp = temp >> 1;
        set_flags(Z, temp == 0);
        set_flags(N | HC, false);

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <uint8_t bit, Register r> void SM83::op_bit()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        switch (bit)
        {
        case 0:
            set_flags(Z, !(temp & 1));
            break;
        case 1:
            set_flags(Z, !(temp & 2));
            break;
        case 2:
            set_flags(Z, !(temp & 4));
            break;
        case 3:
            set_flags(Z, !(temp & 8));
            break;
        case 4:
            set_flags(Z, !(temp & 16));
            break;
        case 5:
            set_flags(Z, !(temp & 32));
            break;
        case 6:
            set_flags(Z, !(temp & 64));
            break;
        case 7:
            set_flags(Z, !(temp & 128));
            break;
        }

        set_flags(N, false);
        set_flags(HC, true);
    }

    template <uint8_t bit, Register r> void SM83::op_res()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        switch (bit)
        {
        case 0:
            temp &= ~(1);
            break;
        case 1:
            temp &= ~(2);
            break;
        case 2:
            temp &= ~(4);
            break;
        case 3:
            temp &= ~(8);
            break;
        case 4:
            temp &= ~(16);
            break;
        case 5:
            temp &= ~(32);
            break;
        case 6:
            temp &= ~(64);
            break;
        case 7:
            temp &= ~(128);
            break;
        }

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }

    template <uint8_t bit, Register r> void SM83::op_set()
    {
        uint8_t temp = registers[r];

        if constexpr (r == HL_ADDR)
        {
            temp = read(get_rp(HL));
        }

        switch (bit)
        {
        case 0:
            temp |= (1);
            break;
        case 1:
            temp |= (2);
            break;
        case 2:
            temp |= (4);
            break;
        case 3:
            temp |= (8);
            break;
        case 4:
            temp |= (16);
            break;
        case 5:
            temp |= (32);
            break;
        case 6:
            temp |= (64);
            break;
        case 7:
            temp |= (128);
            break;
        }

        if constexpr (r == HL_ADDR)
        {
            write(get_rp(HL), temp);
        }
        else
        {
            registers[r] = temp;
        }
    }
}
