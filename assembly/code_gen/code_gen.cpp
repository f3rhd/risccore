
#include "code_gen.hpp"

namespace code_gen  {
    static uint32_t encode_r(const Instruction &instr){
         return (instr.func7 << 25) |
           (instr.rs2   << 20) |
           (instr.rs1   << 15) |
           (instr.func3 << 12) |
           (instr.rd    << 7)  |
           (instr.opcode);
    }
    static uint32_t encode_i(const Instruction& instr ){

        uint32_t imm = static_cast<uint32_t>(instr.imm) & 0xFFF;  // 12-bit immediate
        return (imm << 20) |
            ((instr.rs1   & 0x1F) << 15) |
            ((instr.func3 & 0x07) << 12) |
            ((instr.rd    & 0x1F) << 7)  |
            ((instr.opcode & 0x7F));

    }
    static uint32_t encode_b(const Instruction &instr){
        uint32_t imm = static_cast<uint32_t>(instr.imm);

        // imm[12] -> bit 31
        uint32_t imm_12 = (imm >> 12) & 0x1;
        // imm[10:5] -> bits 30:25
        uint32_t imm_10_5 = (imm >> 5) & 0x3F;
        // imm[4:1] -> bits 11:8
        uint32_t imm_4_1 = (imm >> 1) & 0xF;
        // imm[11] -> bit 7
        uint32_t imm_11 = (imm >> 11) & 0x1;

        uint32_t raw = 0;
        raw |= (imm_12 << 31);
        raw |= (imm_10_5 << 25);
        raw |= ((instr.rs2 & 0x1F) << 20);
        raw |= ((instr.rs1 & 0x1F) << 15);
        raw |= ((instr.func3 & 0x7) << 12);
        raw |= (imm_4_1 << 8);
        raw |= (imm_11 << 7);
        raw |= (instr.opcode & 0x7F);
        return raw;
    }
    static uint32_t encode_j(const Instruction &instr){

        uint32_t imm = static_cast<uint32_t>(instr.imm);
        uint32_t imm_20   = (imm >> 20) & 0x1;
        uint32_t imm_10_1 = (imm >> 1) & 0x3FF;
        uint32_t imm_11   = (imm >> 11) & 0x1;
        uint32_t imm_19_12= (imm >> 12) & 0xFF;

        uint32_t raw = 0;
        raw |= (imm_20 << 31);
        raw |= (imm_10_1 << 21);
        raw |= (imm_11 << 20);
        raw |= (imm_19_12 << 12);
        raw |= ((instr.rd & 0x1F) << 7);
        raw |= (instr.opcode & 0x7F);

        return raw;
    }
    static uint32_t encode_u(const Instruction& instr ){

        uint32_t imm = static_cast<uint32_t>(instr.imm) & 0xFFFFF000; // Upper 20 bits, lower 12 zero

        uint32_t raw = 0;
        raw |= imm;
        raw |= ((instr.rd & 0x1F) << 7);
        raw |= (instr.opcode & 0x7F);

        return raw;

    }
    static uint32_t encode_s(const Instruction &instr){
        uint32_t imm = static_cast<uint32_t>(instr.imm);

        uint32_t imm_11_5 = (imm >> 5) & 0x7F;
        uint32_t imm_4_0  = imm & 0x1F;

        uint32_t raw = 0;
        raw |= (imm_11_5 << 25);
        raw |= ((instr.rs2 & 0x1F) << 20);
        raw |= ((instr.rs1 & 0x1F) << 15);
        raw |= ((instr.func3 & 0x7) << 12);
        raw |= (imm_4_0 << 7);
        raw |= (instr.opcode & 0x7F);

        return raw;
    }
    uint32_t encode_p(const Instruction &instruction); //@Incomplete
    static uint32_t encode_instr(const Instruction &instr)
    {
        switch(instr.type){
            case instruction_look_up::OPERATION_TYPE::R_TYPE:
                return encode_r(instr);
            case instruction_look_up::OPERATION_TYPE::I_TYPE:
                return encode_i(instr);
            case instruction_look_up::OPERATION_TYPE::S_TYPE:
                return encode_s(instr);
            case instruction_look_up::OPERATION_TYPE::B_TYPE:
                return encode_b(instr);
            case instruction_look_up::OPERATION_TYPE::J_TYPE:
                return encode_j(instr);
            case instruction_look_up::OPERATION_TYPE::U_TYPE:
                return encode_u(instr);
            case instruction_look_up::OPERATION_TYPE::PSEUDO: //@Incomplete
               // return encode_p(instr);
            case instruction_look_up::OPERATION_TYPE::UNKNOWN:
            default:
                return 0;
        }
    }
    void generate_bin_file(const std::string& output_file_path ,const std::vector<Instruction>& instructions){

        FILE *output_file = fopen(output_file_path.c_str(), "wb");

        for(const Instruction& instr : instructions){
            uint32_t raw = encode_instr(instr);
            fwrite(&raw, sizeof(raw), 1,output_file);
        }
        fclose(output_file);
    }
}