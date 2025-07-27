module pipeline_register_id_ex(
    input logic clk,
    input logic reset,
    input logic enable,
    input logic[24:0] ctrl_signals_in,
    input logic[31:0] read1_in,
    input logic[31:0] read2_in,
    input logic[31:0] imm_in,
    input logic[31:0] pc_in,
    input logic[31:0] pc4_in,
    input logic[4:0] reg_write_addr_in ,
    input logic[4:0] rs1_addr_in,
    input logic[4:0] rs2_addr_in,

    output logic[24:0] ctrl_signals_out,
    output logic[31:0] read1_out,
    output logic[31:0] read2_out,
    output logic[31:0] imm_out,
    output logic[31:0] pc_out,
    output logic[31:0] pc4_out,
    output logic[4:0] reg_write_addr_out,
    output logic[4:0] rs1_addr_out,
    output logic[4:0] rs2_addr_out
);

    always_ff @(posedge clk) begin
        if(reset) begin
            ctrl_signals_out <= '0;
            read1_out <= '0;
            read2_out <= '0;
            imm_out  <= '0;
            pc_out <= '0;
            pc4_out <= '0;
            reg_write_addr_out <= '0;
            rs1_addr_out <= '0;
            rs2_addr_out <= '0;
        end
        else if(enable) begin
            ctrl_signals_out <= ctrl_signals_in;
            read1_out <= read1_in;
            read2_out <= read2_in;
            imm_out  <= imm_in;
            pc_out <= pc_in;
            pc4_out <= pc4_in;
            reg_write_addr_out <= reg_write_addr_in;
            rs1_addr_out <= rs1_addr_in;
            rs2_addr_out <= rs2_addr_in;
        end
    end
endmodule