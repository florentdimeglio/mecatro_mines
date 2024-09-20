close all
dt = 5e-3;
thetaL = unwrap(data_values('thetaL'));
thetaR = unwrap(data_values('thetaR'));

plot(log_time, thetaL - thetaL(1), log_time, thetaR  - thetaR(1))

dataL = iddata(thetaL',data_values('V')',dt);
tfL = tfest(dataL,2,0)

dataR = iddata(thetaR',data_values('V')',dt);
tfR = tfest(dataR,2,0)