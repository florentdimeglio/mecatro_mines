function  [time_data, log_data, line_idx] = ...
    get_data_USB(port_name, Tmax, baudrate, gains, varargin)

if nargin < 4
    gains = 0;
end

CHUNK_SIZE = 10000;
should_terminate = false;
connected = false;
should_connect = true;
should_disconnect = false;
should_terminate = false;
data_to_send = '';
time_data = zeros(1, CHUNK_SIZE);
log_data = containers.Map;
line_idx = -1;
status_message = {'Waiting'};


% Nested functions

    function recv_data = recv_all(client, nbytes)
        n_recv = 0;
        recv_data = [];
        while n_recv < nbytes
            recv_data = [recv_data; fread(client, nbytes - n_recv, 'uint8')];
            n_recv = length(recv_data);
        end
    end

    function set_port_name(new_port_name)
        port_name = new_port_name;
    end

    function set_baudrate(new_baudrate)
        baudrate = new_baudrate;
    end

    function set_folder(folder)
        target_folder = folder;
    end

    function attempt_connection()
        should_connect = true;
    end

    function disconnect()
        should_disconnect = true;
    end

    function terminate()
        should_disconnect = true;
        should_terminate = true;
    end

    function is_connected = check_connected()
        is_connected = connected;
    end

    function [time_out, log_data_out, line_idx_out] = get_data()
        time_out = time_data;
        log_data_out = log_data;
        line_idx_out = line_idx;
    end

    function status = get_status()
        status = status_message;
        status_message = {};
    end

    function send(data)
        data_to_send = [data_to_send data];
    end

while ~should_terminate
    time_data = [];
    log_data = containers.Map;
    line_idx = 0;
    while ~should_connect && ~should_terminate
        pause(0.050);
    end
    if should_terminate
        return;
    end
    should_connect = false;
    should_disconnect = false;

    try
        % Connect to USB
        ser = serialport(port_name, baudrate, 'Timeout', 0.1);
        configureTerminator(ser, "CR/LF");
        write(ser, 's', 'char');
        send_gains_to_robot(gains, ser);
        status_message = {'Connected, waiting for Arduino to boot'};
        connected = true;

        TIMEOUT = 5.0;
        start_time = tic;
        desired_update_period_us = -1;

        while ~should_disconnect && ~should_terminate && toc(start_time) < TIMEOUT
            line_done = false;
            line = '';
            while ~line_done
                %d = char(read(ser, 1, 'char'));
                d = read(ser, 1, 'uint8');
                line = [line d];
                line_done = (d == newline) && (length(line) > 2);
            end
            line = strip(line);
            if ~isempty(line)
                if startsWith(line, "mecatro@")
                    header = strsplit(line, '@');
                    desired_update_period_us = str2double(header{2});

                    log_data = containers.Map();
                    for i = 3:numel(header)
                        log_data(header{i}) = [];
                    end
                    break;
                end
            end
            % if startsWith(line, 'mecatro@')
            %     header = strsplit(extractAfter(line, 'mecatro@'), '@');
            %     desired_update_period_us = str2double(header{1});
            %     log_data = containers.Map;
            %     for i = 2:numel(header)
            %         log_data(header{i}) = zeros(1, CHUNK_SIZE);
            %     end
            %     break;
            % end
            pause(0.01);
        end

        if should_terminate
            return;
        end
        if desired_update_period_us < 0
            status_message{end+1} = 'Timeout on Arduino boot. Is the correct baudrate selected?';
            connected = false;
            break;
        end

        status_message{end+1} = sprintf('Arduino code started. Target period: %dus', desired_update_period_us);
        disp('Arduino code started. Target period: %dus');
        disp(desired_update_period_us);
        max_time = 1.05 * desired_update_period_us / 1e6;

        data_to_send = '';
        while ~should_disconnect
            %write(ser, data_to_send, 'char');
            data_to_send = '';
            % Read data byte by byte until an @ is found
            if read(ser, 1,"uint8") == uint8('@')
                data = read(ser, 4 * (1 + numel(log_data.keys())), 'uint8');
                line_idx = line_idx + 1;

                % Resize buffers as needed
                if line_idx >= numel(time_data)
                    time_data = [time_data, zeros(1, CHUNK_SIZE)];

                    mykeys = log_data.keys();
                    for k = mykeys
                        log_data(k{1}) = [log_data(k{1}), zeros(1, CHUNK_SIZE)];
                    end
                end

                % Decode
                %time_data(line_idx) = typecast(uint8(data(1:4)), 'single') / 1.0e6;
                current_time = typecast(uint8(data(1:4)), 'uint32');
                time_data(line_idx) = cast(current_time,'double') / 1.0e6;
                % byte1 = uint8(data(1));
                % byte2 = uint8(data(2));
                % byte3 = uint8(data(3));
                % byte4 = uint8(data(4));
                %
                % current_time = single(bitshift(byte4, 24) + ...
                %     bitshift(byte3, 16) + bitshift(byte2, 8) + byte1);
                % time(line_idx) = current_time / 1.0e6;

                mykeys = log_data.keys();
                for i = 1:numel(mykeys)
                    key = header{i+2};
                    index = 4 * i + 1;
                    tempValue = typecast(uint8(data(index:index+3)), 'single');
                    tempValue = cast(tempValue,'double');

                    % startIndex = 4 * i;
                    % %endIndex = 4 * (i + 1) - 1;
                    %
                    % byte1 = uint8(data(startIndex));
                    % byte2 = uint8(data(startIndex + 1));
                    % byte3 = uint8(data(startIndex + 2));
                    % byte4 = uint8(data(startIndex + 3));
                    %
                    % tempValue = single(bitshift(byte4, 24) + ...
                    %     bitshift(byte3, 16) + bitshift(byte2, 8) + byte1);
                    currentData = log_data(key);
                    currentData(line_idx) = tempValue;
                    log_data(key) = currentData;
                end
                
                if line_idx > 1
                    dt = time_data(line_idx) - time_data(line_idx - 1);
                else
                    dt = 0;
                end
                if dt > max_time
                    status_message{end+1} = sprintf('Warning: lag detected on the Arduino: iteration duration %dus > %dus.', int32(1e6 * dt), desired_update_period_us);
                end
            end
            if time_data(line_idx) - time_data(1) > Tmax
                should_disconnect = true;
            end
        end

    catch err
        if contains(err.message, 'Access is denied')
            status_message{end+1} = 'ERROR: Access to serial port denied. Is Arduino SerialMonitor running?';
        elseif contains(err.message, 'The device does not recognize the command')
            status_message{end+1} = 'ERROR: Communication with Arduino lost.';
        else
            status_message{end+1} = ['ERROR: serial communication error: ' err.message];
        end
    end

    % status_message{end+1} = 'Arduino disconnected';
    % connected = false;
    if line_idx > 0
        %log_path = fullfile(target_folder, ['log_', datestr(now, 'ddmmyyyy_HHMMSS'), '.csv']);
        time_data = time_data(1:line_idx) - time_data(1);
        key_set = keys(log_data);
        for i = 1:numel(key_set)
            temp_array = log_data(key_set{i});
            temp_array = temp_array(1:line_idx);  % Correct the array size
            log_data(key_set{i}) = temp_array;  % Store the modified array back in the Map
        end
        % fid = fopen(log_path, 'w');
        % fprintf(fid, 'Time,%s\n', strjoin(key_set, ','));
        % for i = 1:line_idx
        %     values_to_print = cell(1, numel(key_set));  % Initialize the cell array
        %     for j = 1:numel(key_set)
        %         log_array = log_data(key_set{j});  % Retrieve the array
        %         values_to_print{j} = num2str(log_array(i));  % Convert the specific value to string
        %     end
        %     fprintf(fid, '%.5f,%s\n', time_data(i), strjoin(values_to_print, ','));
        % end
        % fclose(fid);
        % status_message{end+1} = ['Log saved as: ' log_path];
    end
    should_terminate = true;
end
disp(status_message(end))
end