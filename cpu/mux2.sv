module mux2(
    input logic[31:0] a,b,c,d,
    input logic[1:0]  select,
    output logic[31:0] out
);
    always_comb begin
        case(select)
        2'd0: out = a;
        2'd1: out = b;
        2'd2: out = c;
        2'd3: out = d;
        default:
            out = '0;
        endcase
    end
endmodule