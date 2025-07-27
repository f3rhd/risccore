module hazard_unit(
    input logic[4:0] ex_rs1_addr,
    input logic[4:0] ex_rs2_addr,
    input logic[4:0] ex_rd_addr,
    input logic[4:0] dec_rs1_addr,
    input logic[4:0] dec_rs2_addr,
    input logic[4:0] mem_reg_write_addr,
    input logic[4:0] wb_reg_write_addr,
    input logic[2:0] ex_ram_read_signal,
    input logic mem_reg_write_signal,
    input logic wb_reg_write_signal,
    input logic pc_select,

    output logic[1:0] forward_alu_a,
    output logic[1:0] forward_alu_b,
    output logic     flush_dec_ex_pipeline,
    output logic     stall_pc,
    output logic     stall_fetch_decode_pipeline,
    output logic     flush_fetch_decode_pipeline
);

    logic stall_signal;
    always_comb begin 

        // Forwarding from memory stage
        if(ex_rs1_addr == mem_reg_write_addr && mem_reg_write_signal && ex_rs1_addr != 0) begin
            forward_alu_a = 2'b11;
        end
        // Forward from write_back stage
        else if(ex_rs1_addr == wb_reg_write_addr && wb_reg_write_signal && ex_rs1_addr != 0) begin
            forward_alu_a = 2'b10;
        end
        else 
            forward_alu_a = 2'b00; // no forwarding

        //Forward from memory stage
        if(ex_rs2_addr == mem_reg_write_addr && mem_reg_write_signal && ex_rs2_addr != 0) begin
            forward_alu_b = 2'b11;
        end
        // Forward from write back stage
        else if(ex_rs2_addr == wb_reg_write_addr && wb_reg_write_signal && ex_rs2_addr != 0) begin
            forward_alu_b = 2'b10;
        end
        else 
            forward_alu_b = 2'b00; // no forwarding

        // Load is in execution stage
        stall_signal = ex_ram_read_signal >= 3'd1 && ex_ram_read_signal <= 3'd5 && (dec_rs1_addr == ex_rd_addr || dec_rs2_addr == ex_rd_addr);
        stall_pc = stall_signal;
        stall_fetch_decode_pipeline = stall_signal;
        flush_dec_ex_pipeline = stall_signal || pc_select;
        flush_fetch_decode_pipeline = pc_select;
    end
endmodule