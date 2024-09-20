%Duration of the experiment
Tmax = 10;

%Serial port name
port_name = "/dev/cu.usbserial-1130";

%CONTROL_LOOP_PERIOD constant in the Arduino sketch
dt_arduino = 5e-3;

%Baudrate
baudrate = 1000000;

%Target folder
target_folder = './';

[log_time, data_values, line_idx] = ...
    get_data_from_robot(port_name, Tmax, baudrate);

%Display logged variable names
disp(data_values.keys)  

% Plot logged variables
figure; 
hold on;
legend_labels = data_values.keys;
for i = 1:numel(legend_labels)
    d = legend_labels{i};
    v = data_values(d);
    plot(log_time, v, 'DisplayName', d);
end
hold off;
legend('Location', 'best');
grid on;


