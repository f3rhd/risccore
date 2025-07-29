module rom #(parameter SIZE = (4 << 20)) // 4MB
(
    input  logic [31:0] word_addr, // word address (e.g., PC >> 2)
    output logic [31:0] rom_out
);
    logic [7:0] cells [0:SIZE-1];

    // === Initialization from file ===
    initial begin
        $readmemb("instructions.mem", cells);
    end

    // === Read 32-bit word (little-endian) ===
    always_comb begin
        if (word_addr + 3 < SIZE) begin
            rom_out = {cells[word_addr + 3], cells[word_addr + 2], cells[word_addr + 1], cells[word_addr]};
        end else begin
            rom_out = 32'd0;
        end
    end
endmodule

