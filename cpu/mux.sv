module mux(
    input logic[31:0] a,b,
    input logic select,
    output logic[31:0] out
);
    assign out  = select ? b : a;
endmodule