#include "reader-arm.h"
#include <stdlib.h>

ZzARMReader *zz_arm_reader_new(zz_ptr_t insn_address) {
    ZzARMReader *reader = (ZzARMReader *)zz_malloc_with_zero(sizeof(ZzARMReader));

    reader->r_start_address   = (zz_addr_t)insn_address;
    reader->r_current_address = (zz_addr_t)insn_address;
    reader->start_pc          = (zz_addr_t)insn_address + 8;
    reader->current_pc        = (zz_addr_t)insn_address + 8;
    reader->size              = 0;
    reader->insn_size         = 0;
    return reader;
}

void zz_arm_reader_init(ZzARMReader *self, zz_ptr_t insn_address) { zz_arm_reader_reset(self, insn_address); }

void zz_arm_reader_reset(ZzARMReader *self, zz_ptr_t insn_address) {
    self->r_start_address   = (zz_addr_t)insn_address;
    self->r_current_address = (zz_addr_t)insn_address;
    self->start_pc          = (zz_addr_t)insn_address + 8;
    self->current_pc        = (zz_addr_t)insn_address + 8;
    self->size              = 0;
    self->insn_size         = 0;
}

void zz_arm_reader_free(ZzARMReader *self) {
    if (self->insn_size) {
        for (int i = 0; i < self->insn_size; i++) {
            free(self->insns[i]);
        }
    }
    free(self);
}

ZzARMInstruction *zz_arm_reader_read_one_instruction(ZzARMReader *self) {
    ZzARMInstruction *insn_ctx = (ZzARMInstruction *)zz_malloc_with_zero(sizeof(ZzARMInstruction));
    insn_ctx->type             = ARM_INSN;
    insn_ctx->address          = (zz_addr_t)self->r_current_address;
    insn_ctx->pc               = (zz_addr_t)self->current_pc;
    insn_ctx->insn             = *(uint32_t *)self->r_current_address;
    insn_ctx->size             = 4;

    self->current_pc += insn_ctx->size;
    self->r_current_address += insn_ctx->size;
    self->insns[self->insn_size++] = insn_ctx;
    self->size += insn_ctx->size;
    return insn_ctx;
}

// ARM Manual
// A5 ARM Instruction Set Encoding
// A5.3 Load/store word and unsigned byte
ARMInsnType GetARMInsnType(uint32_t insn) {

    if (insn_equal(insn, "xxxx0000100xxxxxxxxxxxxxxxx0xxxx") && (get_insn_sub(insn, 28, 4) != 0xF)) {
        return ARM_INS_ADD_register_A1;
    }

    if (insn_equal(insn, "xxxx0101x0011111xxxxxxxxxxxxxxxx") && (get_insn_sub(insn, 28, 4) != 0xF)) {
        return ARM_INS_LDR_literal_A1;
    }

    if (insn_equal(insn, "xxxx001010001111xxxxxxxxxxxxxxxx") && (get_insn_sub(insn, 28, 4) != 0xF)) {
        return ARM_INS_ADR_A1;
    }
    if (insn_equal(insn, "xxxx001001001111xxxxxxxxxxxxxxxx") && (get_insn_sub(insn, 28, 4) != 0xF)) {
        return ARM_INS_ADR_A2;
    }
    if (insn_equal(insn, "xxxx1010xxxxxxxxxxxxxxxxxxxxxxxx") && (get_insn_sub(insn, 28, 4) != 0xF)) {
        return ARM_INS_B_A1;
    }
    if (insn_equal(insn, "xxxx1011xxxxxxxxxxxxxxxxxxxxxxxx") && (get_insn_sub(insn, 28, 4) != 0xF)) {
        return ARM_INS_BLBLX_immediate_A1;
    }
    if (insn_equal(insn, "1111101xxxxxxxxxxxxxxxxxxxxxxxxx")) {
        return ARM_INS_BLBLX_immediate_A2;
    }

    return ARM_UNDEF;
}