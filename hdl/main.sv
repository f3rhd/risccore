module cpu(
    input logic clk,
    input logic _reset
);

    const logic zero = 0;
    const logic one = 1;
    const logic[31:0] four = 4;

    logic[31:0] _pc_in;
    logic[31:0] _pc_four_adder_out;
    logic[31:0] _write_stage_result; // is output of write_select module
    logic[31:0] _pc_out;
    logic[31:0] _rom_instr_out;

    logic _pc_select_signal; //is output of  branch module 
    logic _pc_stall; // is output of hazard unit
    logic _stall_if_id_pipeline; // is output of hazard unit
    logic _flush_if_id_pipeline; // is output of hazard unit

    logic[31:0] _pip_ex_mem_pcimm_in;
    adder pc_four_adder(
        .a(_pc_out),
        .b(four),
        .out(_pc_four_adder_out)
    );

    mux pc_select_mux(
        .a(_pc_four_adder_out),
        .b(_pip_ex_mem_pcimm_in),
        .select(_pc_select_signal),
        .out(_pc_in)
    );
    program_counter pc(
        .clk(clk),
        .reset(zero | _reset),
        .enable(~_pc_stall),
        .pc_in(_pc_in),
        .pc_out(_pc_out)
    );
    rom #(256) instr_memory(
        .word_addr(_pc_out),
        .rom_out(_rom_instr_out)
    );

    logic[31:0] _pip_if_id_instr_out;
    logic[31:0] _pip_if_id_pc_out;
    logic[31:0] _pip_if_id_pc4_out;


    pipeline_register_if_id pip_if_id(
        .clk(clk),
        .reset(_flush_if_id_pipeline | _reset),
        .enable(~_stall_if_id_pipeline),
        .instr_in(_rom_instr_out),
        .pc_in(_pc_out),
        .pc4_in(_pc_four_adder_out),
        .instr_out(_pip_if_id_instr_out),
        .pc_out(_pip_if_id_pc_out),
        .pc4_out(_pip_if_id_pc4_out)
    );


    logic[24:0] _ctrl_signals;
    control_unit _control_unit(
        .instr(_pip_if_id_instr_out),
        .ctrl_signals(_ctrl_signals)
    );


    logic[3:0] _pip_mem_wb_ctrl_signals_out; // is output of pip_mem_wb
    logic[31:0] _reg_file_read_data_1;
    logic[31:0] _reg_file_read_data_2;
    logic[4:0] _pip_mem_wb_reg_write_addr_out ; // is output of pip_mem_wb
    reg_file  _reg_file(
        .clk(clk),
        .reset(_reset),
        .write_enable(_pip_mem_wb_ctrl_signals_out[3]),
        .read_addr_1(_pip_if_id_instr_out[19:15]),
        .read_addr_2(_pip_if_id_instr_out[24:20]),
        .write_addr(_pip_mem_wb_reg_write_addr_out),
        .write_data(_write_stage_result),
        .read_data_1(_reg_file_read_data_1),
        .read_data_2(_reg_file_read_data_2)
    );

    logic[31:0] _imm_val_output;
    imm_select _imm_select(
        .instr(_pip_if_id_instr_out),
        .imm_select_ctrl(_ctrl_signals[17:15]),
        .output_imm_val(_imm_val_output)
    );

    logic _flush_id_ex_pipeline; // is output of hazard unit

    logic[24:0] _pip_id_ex_ctrl_signals_out;
    logic[31:0] _pip_id_ex_read1_out;
    logic[31:0] _pip_id_ex_read2_out;
    logic[31:0] _pip_id_ex_imm_out;
    logic[31:0] _pip_id_ex_pc_out;
    logic[31:0] _pip_id_ex_pc4_out;
    logic[4:0] _pip_id_ex_reg_write_addr_out;
    logic[4:0] _pip_id_ex_rs1_addr_out;
    logic[4:0] _pip_id_ex_rs2_addr_out;
    pipeline_register_id_ex pip_id_ex(
        .clk(clk),
        .reset(_flush_id_ex_pipeline | _reset),
        .enable(one),
        .ctrl_signals_in(_ctrl_signals),
        .read1_in(_reg_file_read_data_1),
        .read2_in(_reg_file_read_data_2),
        .imm_in(_imm_val_output),
        .pc_in(_pip_if_id_pc_out),
        .pc4_in(_pip_if_id_pc4_out),
        .reg_write_addr_in(_pip_if_id_instr_out[11:7]),
        .rs1_addr_in(_pip_if_id_instr_out[19:15]),
        .rs2_addr_in(_pip_if_id_instr_out[24:20]),
        .ctrl_signals_out(_pip_id_ex_ctrl_signals_out),
        .read1_out(_pip_id_ex_read1_out),
        .read2_out(_pip_id_ex_read2_out),
        .imm_out(_pip_id_ex_imm_out),
        .pc_out(_pip_id_ex_pc_out),
        .pc4_out(_pip_id_ex_pc4_out),
        .rs1_addr_out(_pip_id_ex_rs1_addr_out),
        .rs2_addr_out(_pip_id_ex_rs2_addr_out),
        .reg_write_addr_out(_pip_id_ex_reg_write_addr_out)
    );

    logic[4:0] _pip_ex_mem_reg_write_addr_out; // is output of pip_ex_mem
    logic[9:0] _pip_ex_mem_ctrl_signals_out;// is output of pip_ex_mem
    logic[1:0] _forward_alu_a;
    logic[1:0] _forward_alu_b;

    hazard_unit _hazard_unit(
        .ex_rs1_addr(_pip_id_ex_rs1_addr_out),
        .ex_rs2_addr(_pip_id_ex_rs2_addr_out),
        .ex_rd_addr(_pip_id_ex_reg_write_addr_out),
        .dec_rs1_addr(_pip_if_id_instr_out[19:15]),
        .dec_rs2_addr(_pip_if_id_instr_out[24:20]),
        .mem_reg_write_addr(_pip_ex_mem_reg_write_addr_out),
        .mem_reg_write_signal(_pip_ex_mem_ctrl_signals_out[8]),
        .wb_reg_write_signal(_pip_mem_wb_ctrl_signals_out[3]),
        .wb_reg_write_addr(_pip_mem_wb_reg_write_addr_out),
        .ex_ram_read_signal(_pip_id_ex_ctrl_signals_out[7:5]),
        .pc_select(_pc_select_signal),
        .forward_alu_a(_forward_alu_a),
        .forward_alu_b(_forward_alu_b),
        .flush_dec_ex_pipeline(_flush_id_ex_pipeline),
        .stall_pc(_pc_stall),
        .stall_fetch_decode_pipeline(_stall_if_id_pipeline),
        .flush_fetch_decode_pipeline(_flush_if_id_pipeline)
    );

    logic[31:0] _pip_ex_mem_alu_result_out;
    logic[31:0] _alu_a_in;
    logic[31:0] _alu_b_forward;
    mux2 alu_a_select(
        .a(_pip_id_ex_read1_out),
        .b(32'd0),
        .c(_write_stage_result),
        .d(_pip_ex_mem_alu_result_out),
        .select(_forward_alu_a),
        .out(_alu_a_in)
    );

    mux2 alu_forward_select(
        .a(_pip_id_ex_read2_out),
        .b(32'd0),
        .c(_write_stage_result),
        .d(_pip_ex_mem_alu_result_out),
        .select(_forward_alu_b),
        .out(_alu_b_forward)
    );

    logic[31:0] _alu_b_in;
    mux alu_b_select(
        .a(_alu_b_forward),
        .b(_pip_id_ex_imm_out),
        .select(_pip_id_ex_ctrl_signals_out[13]),
        .out(_alu_b_in)
    );
    

    logic[31:0] _alu_result;
    logic[2:0]  _comparison_flags;
    alu _alu(
        .alu_a(_alu_a_in),
        .alu_b(_alu_b_in),
        .alu_operation_type(_pip_id_ex_ctrl_signals_out[11:8]),
        .comparison_mode(_pip_id_ex_ctrl_signals_out[12]),
        .alu_result(_alu_result),
        .comparison_flags(_comparison_flags)
    );
    branch _branch(
        .alu_flags(_comparison_flags),
        .control_flags(_pip_id_ex_ctrl_signals_out[24:18]),
        .should_branch(_pc_select_signal)
    );
    
    adder _adder(
        .a(_pip_id_ex_imm_out),
        .b(_pip_id_ex_pc_out),
        .out(_pip_ex_mem_pcimm_in)
    );

    logic[31:0] _pip_ex_mem_lt_sgn_ext_out;
    logic[31:0] _pip_ex_mem_ram_data_out;
    logic[31:0] _pip_ex_mem_imm_out;
    logic[31:0] _pip_ex_mem_pcimm_out;// pc+imm
    logic[31:0] _pip_ex_mem_pc4_out;
    pipeline_register_ex_mem pip_ex_mem(
        .clk(clk),
        .reset(zero | _reset),
        .enable(one),
        .ctrl_signals_in({1'b0,_pip_id_ex_ctrl_signals_out[14],_pip_id_ex_ctrl_signals_out[7:5],_pip_id_ex_ctrl_signals_out[4:3],_pip_id_ex_ctrl_signals_out[2:0]}),
        .lt_sgn_ext_in({32{_comparison_flags[2]}}),
        .alu_result_in(_alu_result),
        .ram_data_in(_alu_b_forward),
        .imm_in(_pip_id_ex_imm_out),
        .pcimm_in(_pip_ex_mem_pcimm_in),
        .pc4_in(_pip_id_ex_pc4_out),
        .reg_write_addr_in(_pip_id_ex_reg_write_addr_out),

        .ctrl_signals_out(_pip_ex_mem_ctrl_signals_out),
        .lt_sgn_ext_out(_pip_ex_mem_lt_sgn_ext_out),
        .alu_result_out(_pip_ex_mem_alu_result_out),
        .ram_datain_out(_pip_ex_mem_ram_data_out),
        .imm_out(_pip_ex_mem_imm_out),
        .pcimm_out(_pip_ex_mem_pcimm_out),
        .pc4_out(_pip_ex_mem_pc4_out),
        .reg_write_addr_out(_pip_ex_mem_reg_write_addr_out)
    );

    logic[31:0] _ram_out;
    ram #(256) data_memory(
        .clk(clk),
        .word_addr(_pip_ex_mem_alu_result_out),
        .data_in(_pip_ex_mem_ram_data_out),
        .write_ctrl(_pip_ex_mem_ctrl_signals_out[4:3]),
        .read_ctrl(_pip_ex_mem_ctrl_signals_out[7:5]),
        .ram_out(_ram_out)
    );

    logic[31:0] _pip_mem_wb_lt_sgn_ext_out;
    logic[31:0] _pip_mem_wb_ram_output_out;
    logic[31:0] _pip_mem_wb_alu_result_out;
    logic[31:0] _pip_mem_wb_imm_out;
    logic[31:0] _pip_mem_wb_pcimm_out;
    logic[31:0] _pip_mem_wb_pc4_out;
    pipeline_register_mem_wb pip_mem_wb(
        .clk(clk),
        .reset(zero | _reset),
        .enable(one),
        .ctrl_signals_in({_pip_ex_mem_ctrl_signals_out[8],_pip_ex_mem_ctrl_signals_out[2:0]}),
        .lt_sgn_ext_in(_pip_ex_mem_lt_sgn_ext_out),
        .ram_output_in(_ram_out),
        .alu_result_in(_pip_ex_mem_alu_result_out),
        .imm_in(_pip_ex_mem_imm_out),
        .pcimm_in(_pip_ex_mem_pcimm_out),
        .pc4_in(_pip_ex_mem_pc4_out),
        .reg_write_addr_in(_pip_ex_mem_reg_write_addr_out),   

        .ctrl_signals_out(_pip_mem_wb_ctrl_signals_out),
        .lt_sgn_ext_out(_pip_mem_wb_lt_sgn_ext_out),
        .ram_output_out(_pip_mem_wb_ram_output_out),
        .alu_result_out(_pip_mem_wb_alu_result_out),
        .imm_out(_pip_mem_wb_imm_out),
        .pcimm_out(_pip_mem_wb_pcimm_out),
        .pc4_out(_pip_mem_wb_pc4_out),
        .reg_write_addr_out(_pip_mem_wb_reg_write_addr_out)
     );

     mux3 write_select(
        .in0(32'd0),
        .in1(32'd0),
        .in2(_pip_mem_wb_lt_sgn_ext_out),
        .in3(_pip_mem_wb_alu_result_out),
        .in4(_pip_mem_wb_imm_out),
        .in5(_pip_mem_wb_ram_output_out),
        .in6(_pip_mem_wb_pc4_out),
        .in7(_pip_mem_wb_pcimm_out),
        .select(_pip_mem_wb_ctrl_signals_out[2:0]),
        .out(_write_stage_result)
     );
endmodule