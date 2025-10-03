module pipeline_register_mem_wb(
    input logic clk,
    input logic reset,
    input logic enable,
    input logic[3:0] ctrl_signals_in,
    input logic[31:0] lt_sgn_ext_in,
    input logic[31:0] ram_output_in,
    input logic[31:0] alu_result_in,
    input logic[31:0] imm_in,
    input logic[31:0] pcimm_in,
    input logic[31:0] pc4_in,
    input logic[4:0] reg_write_addr_in,
    output logic[3:0] ctrl_signals_out,
    output logic[31:0] lt_sgn_ext_out,
    output logic[31:0] ram_output_out,
    output logic[31:0] alu_result_out,
    output logic[31:0] imm_out,
    output logic[31:0] pcimm_out,
    output logic[31:0] pc4_out,
    output logic[4:0] reg_write_addr_out
);
    always_ff @(posedge clk) begin
        if(reset) begin
            ctrl_signals_out <= '0;
            lt_sgn_ext_out <= '0;
            alu_result_out <= '0;
            ram_output_out <= '0;
            imm_out <= '0;
            pcimm_out <= '0;
            pc4_out <= '0;
            reg_write_addr_out <= '0;
        end
        else if(enable) begin
            ctrl_signals_out <= ctrl_signals_in;
            lt_sgn_ext_out <= lt_sgn_ext_in;
            alu_result_out <= alu_result_in;
            ram_output_out <= ram_output_in;
            imm_out <= imm_in;
            pcimm_out <= pcimm_in;
            pc4_out <= pc4_in;
            reg_write_addr_out <= reg_write_addr_in;
        end
    end
endmodule