module stimulus;
	reg pi0, pi1, pi2, pi3;
	wire po0, po1;
	reg[5:0] tmp;
	integer fp_r, fp_w, temp;
    // integer i;
	main_circuit main_1(
		.pi0(pi0),
		.pi1(pi1),
		.pi2(pi2),
		.pi3(pi3),
		.po0(po0),
		.po1(po1),
	);

	initial begin
		fp_r = $fopen("test_pattern", "r");
		fp_w = $fopen("test_circuit_verify.txt", "w");
		// for(i = 0; i < 2; i = i + 1) begin
		// 	tmp = $fgetc(fp_r);
		// 	while(tmp != 10) begin
		// 		tmp = $fgetc(fp_r);
		// 	end
		// end
		while(!$feof(fp_r)) begin
			temp = $fscanf(fp_r, "%d %d %d %d ", pi0, pi1, pi2, pi3);
			#1;
			$fwrite(fp_w, "%d %d \n", po0, po1);
		end
		$fclose(fp_r);
		$fclose(fp_w);
	end
endmodule