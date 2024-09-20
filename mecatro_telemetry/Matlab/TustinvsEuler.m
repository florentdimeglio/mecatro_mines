close all

dt = 5e-3; T = 10;
t = 0:dt:T;
y = sin(2 * pi * t);

tau = 0.03;
a = 2 * tau / dt;

dyf_tustin = zeros(1,length(t));
yfiltered = dyf_tustin;
dyf_euler = dyf_tustin;
dy_tustin = dyf_tustin;
dy_euler = diff(y)/dt;

for i = 1:(length(t)-1)
    dyf_tustin(i+1) =  (1 / tau * (a * y(i+1) - a * y(i)) - (1 - a) * dyf_tustin(i)) / (1 + a);
    yfiltered(i+1) = yfiltered(i) + dt *(y(i+1) - yfiltered(i)) / tau;
    dyf_euler(i+1) = (y(i+1) - yfiltered(i+1)) / tau;
    dy_tustin(i+1) = -dy_tustin(i)+2/dt*(y(i+1)-y(i));
end

figure, subplot(211)
plot(t,cos(t),'k-')
plot(t,dyf_tustin), hold on, grid on
plot(t,dyf_euler)

subplot(212)
plot(t,dy_tustin), hold on, grid on
plot(t(2:end),dy_euler)
plot(t,cos(t),'k-')


