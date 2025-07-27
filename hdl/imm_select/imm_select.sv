module imm_select(
    input logic [31:0] instr,
    input logic [2:0] imm_select_ctrl,
    output logic [31:0] output_imm_val
);
    always_comb begin
        case (imm_select_ctrl)
            3'b000: begin // I-type immediate
                automatic logic [11:0] i_imm = instr[31:20];
                output_imm_val = { {20{i_imm[11]}}, i_imm };
            end

            3'b001: begin // J-type immediate
                logic [20:0] j_imm;
                j_imm[20]    = instr[31];
                j_imm[19:12] = instr[19:12];
                j_imm[11]    = instr[20];
                j_imm[10:1]  = instr[30:21];
                j_imm[0]     = 1'b0;
                output_imm_val = { {11{j_imm[20]}}, j_imm };
            end

            3'b010: begin // U-type immediate
                logic [31:0] u_type;
                u_type[31:12] = instr[31:12];
                u_type[11:0]  = 12'd0;
                output_imm_val = u_type;
            end

            3'b100: begin // S-type immediate
                automatic logic [11:0] s_imm = {instr[31:25], instr[11:7]};
                output_imm_val = { {20{s_imm[11]}}, s_imm };
            end

            3'b111: begin // B-type immediate
                logic [12:0] b_imm;
                b_imm[12:11] = {instr[31], instr[7]};
                b_imm[10:5]  = instr[30:25];
                b_imm[4:1]   = instr[11:8];
                b_imm[0]     = 1'b0;
                output_imm_val = { {19{b_imm[12]}}, b_imm };
            end

            default: output_imm_val = 32'd0;
        endcase
    end
endmodule
