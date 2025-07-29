`timescale 1ps / 1ps

module main_tb;
    logic clk = 0;
    logic reset;

    // Generate clock with 10ns period (10,000ps)
    always #5000 clk = ~clk;

    initial begin
        reset = 1;     // assert reset
        #15000;        // wait 15ns = 1.5 clock cycles
        reset = 0;     // deassert reset
    end

    cpu _cpu(clk, reset);
endmodule
