Point(1) = {1.571,1.571, 0, 1e+22};
Point(2) = {-1.571,1.571, 0, 1e+22};
Point(3) = {-1.571,-1.571, 0, 1e+22};
Point(4) = {1.571,-1.571 , 0, 1e+22};
Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};
Line Loop(10) = {1,2,3,4};
Plane Surface(11) = {10};
Physical Line(102) = {1};
Physical Line(103) = {2};
Physical Line(100) = {3};
Physical Line(101) = {4};
Physical Surface(150) = {11};
Transfinite Line {1,2,3,4} = 5 Using Progression 1;
Transfinite Surface {11};
Recombine Surface {11};