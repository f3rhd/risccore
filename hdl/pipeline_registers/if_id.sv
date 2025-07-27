module pipeline_register_if_id(
    input logic clk,
    input logic reset,
    input logic enable,
    input logic[31:0] instr_in,
    input logic[31:0] pc_in,
    input logic[31:0] pc4_in, // stands for pc + 4
    output logic[31:0] instr_out,
    output logic[31:0] pc_out,
    output logic[31:0] pc4_out // stands for pc + 4
);

    always_ff @(posedge clk) begin
        if(reset) begin
            instr_out <= 32'b0;
            pc_out <= 32'b0;
            pc4_out <= 32'b0;
        end
        else if(enable) begin
            instr_out <= instr_in;
            pc_out <= pc_in;
            pc4_out <= pc4_in;
        end
    end
endmodule