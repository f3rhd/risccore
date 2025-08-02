module ram #(parameter SIZE = (4<<20) // 4MB
)(
    input logic clk,
    input logic [31:0] word_addr,
    input logic [31:0] data_in,
    input logic [1:0] write_ctrl,
    input logic [2:0] read_ctrl,
    output logic [31:0] ram_out
);
    logic [7:0] cells [0:SIZE-1];
    localparam int ADDR_WIDTH = $clog2(SIZE);

    always_comb begin
        automatic logic [ADDR_WIDTH-1:0] _word_addr = word_addr[ADDR_WIDTH-1:0];
        case (read_ctrl)
            3'b001: begin // read word
                if (_word_addr + 3 < SIZE) begin
                    ram_out = {cells[_word_addr + 3], cells[_word_addr + 2], cells[_word_addr + 1], cells[_word_addr]};
                end else
                    ram_out = 32'd0;
            end

            3'b010: begin // read half word zero extend
                if (_word_addr + 1 < SIZE) begin
                    ram_out = {16'd0, cells[_word_addr + 1], cells[_word_addr]};
                end else
                    ram_out = 32'd0;
            end

            3'b011: begin // read half word sign extend
                automatic logic [15:0] half_word = {cells[_word_addr + 1], cells[_word_addr]};
                if (_word_addr + 1 < SIZE) begin
                    ram_out = { {16{half_word[15]}}, half_word };
                end else
                    ram_out = 32'd0;
            end

            3'b100: begin // read byte zero extend
                ram_out = {24'd0, cells[_word_addr]};
            end

            3'b101: begin // read byte sign extend
                automatic logic [7:0] _byte = cells[_word_addr];
                ram_out = { {24{_byte[7]}}, _byte };
            end

            default:
                ram_out = 32'd0;
        endcase
    end

    always_ff @(posedge clk) begin
        automatic logic [ADDR_WIDTH-1:0] _word_addr = word_addr[ADDR_WIDTH-1:0];
        case (write_ctrl)
            2'b11: begin // write word
                
                if (_word_addr < SIZE)
                    cells[_word_addr] <= data_in[7:0];

                if (_word_addr + 1 < SIZE)
                    cells[_word_addr + 1] <= data_in[15:8];

                if (_word_addr + 2 < SIZE)
                    cells[_word_addr + 2] <= data_in[23:16];

                if (_word_addr + 3 < SIZE)
                    cells[_word_addr + 3] <= data_in[31:24];
            end

            2'b01: begin // write half word
                if (_word_addr < SIZE)
                    cells[_word_addr] <= data_in[7:0];

                if (_word_addr + 1 < SIZE)
                    cells[_word_addr + 1] <= data_in[15:8];
            end

            2'b10: begin // write byte
                if (_word_addr < SIZE)
                    cells[_word_addr] <= data_in[7:0];
            end

            default: ; // no write
        endcase
    end

endmodule
