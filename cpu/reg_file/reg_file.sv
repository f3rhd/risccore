module reg_file(
    input logic clk,
    input logic reset,
    input logic write_enable,
    input logic[4:0] read_addr_1,
    input logic[4:0] read_addr_2,
    input logic[4:0] write_addr,
    input logic[31:0] write_data,
    output logic[31:0] read_data_1,
    output logic[31:0] read_data_2
);
    logic[31:0] registers[31:0];
    assign read_data_1 = registers[read_addr_1];
    assign read_data_2 = registers[read_addr_2];
    always_ff @(negedge clk) begin 
        if(reset) begin
            for(int i = 0; i < 32; i++) begin
                registers[i] <= 32'd0;
            end
        end
        else if(write_enable && write_addr != 5'd0) begin 
            $display("Wrote 0x%0h to the register 0x%0h at time:%0d",write_data,write_addr,$time);
            registers[write_addr] <= write_data;
        end
    end
endmodule