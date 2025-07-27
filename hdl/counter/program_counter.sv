module program_counter(
    input  logic        clk,
    input  logic        reset,
    input  logic        enable,
    input  logic [31:0] pc_in,
    output logic [31:0] pc_out
);

    logic [31:0] pc;

    always_ff @(posedge clk or posedge reset) begin
        if (reset)
            pc <= 32'd0;
        else if (enable)
            pc <= pc_in;
    end

    assign pc_out = pc;

endmodule
