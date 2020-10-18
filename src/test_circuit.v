//Just for test
module top (
            pi0, pi1, pi2, pi3 
            po0, po1);
input pi0, pi1, pi2, pi3;
output po0, po1;
wire w0, w1, w2, w3, w4, w5, w6;
assign w0 = (pi0 & w1) | (pi0 & pi1) | (w1 & pi1);
assign w1 = (pi0 & pi2) | (pi0 & pi1) | (pi2 & pi1);
assign w2 = (w1 & pi2) | (w1 & pi3) | (pi2 & pi3);
assign w3 = (pi1 & w0) | (pi1 & w1) | (w0 & w1);
assign w4 = (pi1 & w1) | (pi1 & pi3) | (w1 & pi3);
assign w5 = (w3 & pi0) | (w3 & w4) | (pi0 & w4);
assign w6 = (w0 & w1) | (w0 & w2) | (w1 & w2);
assign po0 = w5;
assign po1 = w6;
endmodule
