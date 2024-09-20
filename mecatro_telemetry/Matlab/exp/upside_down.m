n=30;               %Reduction ratio
mb = 1; 
%Mass of the body [kg]
mw = .1;            %Mass of one wheel [kg]
rho = 0.048;        %Wheel radius [m]
J = 2e-3;           %Moment of inertia of the wheel / gear / motor [kg.m^2]
Ithetabar = 1e-4;  %Moment of inertia of the pitch angle
Ipsibar = 1e-3;    %Moment of inertia of the yaw angle
d = 15e-2;          %Vertical distance from center of wheels to COM [m]
%e = 0e-2;           %Horizontal distance from center of wheels to COM [m]
L = 18e-2;          %Distance between the two wheels [m]
k = 0.35;           %Motor electromagnetic constant [V/rad.s]
Lm = 4e-3;            %Motor inductance [H]
Rm = 2;               %Motor resistance [Ohm]
g = 9.81;           %Gravity constant [m/s2]
Li = 3e-2;          %Horizontal distance from IMU to central axis [m]
di = 20e-2;         %Vertical distance from IMU to wheels axis [m]
g = 9.81;           %Gravity constant [m/s2]
mahony_tau = 1.5;
ka_ = 0.2 / mahony_tau; 

M = mb + 2 * mw + 2 * J / rho^2;                  %Equivalent mass [kg]
Itheta = Ithetabar + mb * d^2;            %Equivalent Moment of Inertia [kg.m2]
%Itheta = Ithetabar + mb * (d^2 + e^2);            
Ipsi = Ipsibar + (mw * rho^2 + J) / 2 / rho^2 * L^2;  %Equivalent Moment of Inertia [kg.m2]
dt = 1e-4;          %Time step for simulations
% thetabar = -atan(e / d);
% D = d * cos(thetabar) - e * sin(thetabar);

% Initial condition
x0 = [0;0;0;0;0;0];
pHat0 = x0(1);
thetaHat0 = x0(2);
uHat0 = x0(4);
wHat0 = x0(5);

%% Linearization

inertiaBar = [M, mb*d, 0;
            mb*d, Itheta, 0;
                0, 0, Ipsi];

% (p,theta,u,w) subsystem with sigmaU as input
inertiaBarUW = inertiaBar(1:2,1:2);
mat1 = inertiaBarUW\[0,0,-1/rho*k^2/Rm*2/rho,1/rho*k^2/Rm*2;0,mb*d*g,k^2/Rm*2/rho,-k^2/Rm*2]; % [u_dot;w_dot] with respect to (p,theta,u,w)
mat2 = inertiaBarUW\[1/rho*k/Rm;-k/Rm];   % [u_dot;w_dot] with respect to sigmaU
Al = [0,0,1,0;0,0,0,1;mat1];
Bl = [0;0;mat2];

% transformation to flat coordinates (mu,theta,mudot,w)
CC = [rho*inertiaBarUW(1,1)+inertiaBarUW(1,2),rho*inertiaBarUW(1,2)+inertiaBarUW(2,2),0,0;0,1,0,0;0,0,rho*inertiaBarUW(1,1)+inertiaBarUW(1,2),rho*inertiaBarUW(1,2)+inertiaBarUW(2,2);0,0,0,1];
Al_CC = CC*Al*inv(CC);
Bl_CC = CC*Bl;

% reduction slow fast
Al_CC_red = Al_CC(1:3,1:3);
Al_CC_red(2,:) = [0,-Al_CC(4,2)/Al_CC(4,4),-Al_CC(4,3)/Al_CC(4,4)];

Bl_CC_red = [0;Bl_CC(4) / (-Al_CC(4,4));0];
%% Estimation

% observer of (theta,w) based on mij
alpha = mb*d*g/(rho*inertiaBarUW(1,1)+inertiaBarUW(1,2))-g;
beta = di-(rho*inertiaBarUW(1,2)+inertiaBarUW(2,2))/(rho*inertiaBarUW(1,1)+inertiaBarUW(1,2));
T = 0.1*10;
w0 = 2*pi/T;
xi = 1;
k1 = -beta/alpha*w0^2;
k2 = 2*xi*w0;

% movie-based observer of (p,theta,xi) with xi = u + di * w 
K_mov = place([0,0,1;0,0,0;0,g,0]',[-1/rho,-1,0]',[-5+1i, -5, -5-1i]);
k1_mov = K_mov(1);  
k2_mov = K_mov(2);
k3_mov = K_mov(3);


%% Control

deriv_tau = 1/100; % for filtered derivative

% State-feedback
K = place(Al,Bl,[-6,-7,-8,-9]);

K = K * 0;
% Measurement-based feedback
% KP theta + KD w + KP_PHI (1/rho p - theta) + KD_PHI (1/rho u - w)
KP_PHI = -rho*K(1);
KD_PHI = -rho*K(3);
KP = -K(2) + KP_PHI;
KD = -K(4) + KD_PHI;

% Observer-based feedback with observer of (p,theta,xi)
Kp = -K(1);
Kth = -K(2);
Kxi = -K(3);
Kw = (-K(4)-Kxi*di)*0;



