% thetaL = data_values('thetaL');
% thetaR = data_values('thetaR');
% dutyCycle = data_values('dutyCycle');

%Load experiment data
load exp/240909-ID.mat

%Convert dutyCycle to Voltage
U = dutyCycle * 12;

%Data for left motor for estimation
dataL = iddata(thetaL', U', 5e-3);

%Data for right motor for estimation
dataR = iddata(thetaR', U', 5e-3);

%Estimate both transfer functions with 2 poles and 1 zero. 
tfL = tfest(dataL, 2, 0);
tfR = tfest(dataR, 2, 0);

