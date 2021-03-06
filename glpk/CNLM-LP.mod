
set N;
/* nodes*/

param r{x in N, y in N}, default 0;
/* residual fund of p.c. (u,v) */ 

param sendingFee{x in N, y in N};
/* fee for sending a payment on p.c. x->y  - kept by the sender */ 


param receivingFee{x in N, y in N};
/* fee for receiving a payment on p.c. x->y - kept by the receiver before forwarding the payment */ 

param source in N;
/* source */

param destination in N;
/* destination */

param P;
/* payment to be made to destination */

var flow{x in N, y in N} >= 0;
/* variable expressing the amount paid by x to y */

s.t. capcon{x in N, y in N}: flow[x,y] <= r[x,y];
/* capacity constraint */

s.t. flocon{z in N: z <> source and z <> destination}: sum{x in N} (flow[x,z] - receivingFee[x,z] * flow[x,z]) = 
					 sum{k in N} (flow[z,k] + sendingFee[z,k]*flow[z,k]);
/* flow conservation constraint */

s.t. endToEnd: sum{x in N: x <> source} flow[source,x]  = P + sum{x in N, y in N}(receivingFee[x,y] * flow[x,y]) +
														 sum{x in N, y in N: x <> source} flow[x,y]*sendingFee[x,y]; 

s.t. endToEnd2: sum{x in N: x <> destination} flow[x, destination] = P;
/* end-to-end flow constraints */


minimize cost: sum{x in N} flow[source,x] - sum{x in N} flow[x,destination];
/* minimizing the amount of fees */

end;

 

