module alu(input logic[31:0] alu_a, 
            input logic[31:0] alu_b,
            input logic[3:0] alu_operation_type,
            input logic comparison_mode, 
            output logic[31:0] alu_result,
            output logic[2:0] comparison_flags
);
    always_comb begin
        
        case(alu_operation_type)
            4'b0000 : alu_result = alu_a ^ alu_b;
            4'b0001 : alu_result  = alu_a & alu_b;
            4'b0010 : alu_result = alu_a + alu_b;
            4'b0011 : alu_result = alu_a << alu_b;
            4'b0100 : alu_result = alu_a >> alu_b; 
            4'b0101 : alu_result = alu_a >>> alu_b;
            4'b0110 : alu_result = alu_a - alu_b;
            4'b0111 : alu_result = alu_a | alu_b;
            4'b1000 : begin 
                alu_result = {$signed(alu_a) * $signed(alu_b)}[31:0];
                $display("ALU(%0x,%0x) = (%x)",alu_a,alu_b,alu_result);
            end
            4'b1001 : alu_result = (alu_b == 32'd0) ? 32'd0 : $signed(alu_a) / $signed(alu_b);
            4'b1010 : alu_result = (alu_b == 32'd0) ? 32'd0 : alu_a / alu_b;
            4'b1011 : alu_result = (alu_b == 32'd0) ? 32'd0 : $signed(alu_a) % $signed(alu_b);
            4'b1100 : alu_result = (alu_b == 32'd0) ? 32'd0 : alu_a % alu_b;
            4'b1101 : alu_result = {$signed(alu_a) * $signed(alu_b)}[63:32];
            4'b1110 : alu_result = {$signed(alu_a) * alu_b}[63:32];
            4'b1111 : alu_result = {alu_a* alu_b}[63:32];
            default : alu_result = 32'd0;
        endcase
        comparison_flags[0] = (alu_a == alu_b);
        if(comparison_mode == 1'b1) begin
            comparison_flags[2] = alu_a < alu_b;
            comparison_flags[1] = alu_a > alu_b;
        end
        else begin 
            comparison_flags[2] = $signed(alu_a) < $signed(alu_b);
            comparison_flags[1] = $signed(alu_a) > $signed(alu_b);
        end
    end
endmodule