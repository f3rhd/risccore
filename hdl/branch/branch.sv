module branch(
    input logic[2:0] alu_flags,
    input logic[6:0] control_flags,
    output logic should_branch
);

    always_comb begin
        should_branch  = control_flags[6] | (control_flags[5] & alu_flags[0]) 
        | (control_flags[4] & (~alu_flags[0])) |(control_flags[3] & alu_flags[2]) 
        |(control_flags[2] & (control_flags[1] | control_flags[0])) | (control_flags[1] & alu_flags[2]) 
        | (control_flags[0] & (alu_flags[1] | alu_flags[0]) );
    end
endmodule