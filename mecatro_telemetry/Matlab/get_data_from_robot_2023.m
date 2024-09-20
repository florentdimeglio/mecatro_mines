function [time, log_data, line_idx] = ...
    get_data_from_robot_2023(port_name, Tmax, baudrate)


CHUNK_SIZE = 10000;

should_terminate = false;
should_connect = false;

while ~should_terminate
    time = zeros(1, CHUNK_SIZE);
    log_data = containers.Map();
    line_idx = 0;

    % Wait for connection order
    % while ~should_connect && ~should_terminate
    %     pause(0.050);
    % end

    if should_terminate
        return;
    end

    % should_connect = false;
    should_disconnect = false;

    % Try to connect
    start_time = 0;

    try
        ser = serialport(port_name, baudrate, 'Timeout', 0.1);
        configureTerminator(ser, "CR/LF");
        status_message = {'Connected, waiting for Arduino to boot'};
        connected = true;

        % Wait for arduino to boot
        TIMEOUT = 5.0;
        start_time = tic;
        desired_update_period_us = -1;

        while ~should_disconnect && ~should_terminate && toc(start_time) < TIMEOUT
            line = readline(ser);
            
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

            pause(0.01);
        end

        if should_terminate
            fclose(ser);
            delete(ser);
            return;
        end

        if desired_update_period_us < 0
            status_message = ['Timeout on Arduino boot. Is the correct baudrate selected ?'];
            connected = false;
            fclose(ser);
            delete(ser);
            break;
        end

        status_message = {sprintf('Arduino code started. Target period: %dus', desired_update_period_us)};
        max_time = 1.05 * desired_update_period_us / 1e6;

        %data_to_send = '';

        while ~should_disconnect
            % Send data.
            %writeline(ser, data_to_send);
            %data_to_send = '';

            % Read data byte by byte until an @ is found
            if read(ser, 1,"uint8") == uint8('@')
                data = read(ser, 4 * (1 + numel(log_data.keys())), 'uint8');
                line_idx = line_idx + 1;

                % Resize buffers as needed
                if line_idx >= numel(time)
                    time = [time, zeros(1, CHUNK_SIZE)];

                    keys = log_data.keys();
                    for k = keys
                        log_data(k{1}) = [log_data(k{1}), zeros(1, CHUNK_SIZE)];
                    end
                end

                % Decode
                %time(line_idx) = typecast(uint8(data(1:4)), 'single') / 1.0e6;
                current_time = typecast(uint8(data(1:4)), 'uint32');
                time(line_idx) = cast(current_time,'double') / 1.0e6;
                % byte1 = uint8(data(1));
                % byte2 = uint8(data(2));
                % byte3 = uint8(data(3));
                % byte4 = uint8(data(4));
                % 
                % current_time = single(bitshift(byte4, 24) + ...
                %     bitshift(byte3, 16) + bitshift(byte2, 8) + byte1);
                % time(line_idx) = current_time / 1.0e6;

                keys = log_data.keys();
                for i = 1:numel(keys)
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

                % Check arduino speed
                dt = time(line_idx+1) - time(line_idx);

                if dt > max_time
                    status_message = {sprintf('Warning: lag detected on the Arduino: iteration duration %dus > %dus.', int32(1e6 * dt), desired_update_period_us)};
                end
            end
            if time(line_idx) > Tmax
                should_disconnect = true;
            end
        end

        clear ser

    catch ME

        if strcmp(ME.identifier, 'MATLAB:serial:fopen:opfailed')
            status_message = {'ERROR: Access to serial port denied. Is Arduino SerialMonitor running ?'};
        elseif strcmp(ME.identifier, 'MATLAB:serial:fread:unsuccessfulRead')
            status_message = {'ERROR: Communication with Arduino lost.'};
        else
            status_message = {['ERROR: serial communication error: ', ME.message]};
        end
        break;
    end

    status_message = {'Arduino disconnected'};
    connected = false;

    if line_idx > 0
        %log_path = fullfile(target_folder, ['log_', datestr(now, 'ddmmyyyy_HHMMSS'), '.csv']);

        % Crop log
        time = time(1:line_idx);

        keys = log_data.keys();
        for i = 1:numel(keys)
            key = keys{i};
            currentData = log_data(key);
            log_data(key) = currentData(1:line_idx);
        end
        % 
        % fid = fopen(log_path, 'w');
        % fprintf(fid, 'Time,%s\n', strjoin(keys, ','));
        % 
        % for i = 1:line_idx
        %     data = cellfun(@(x) sprintf('%.5f', x), num2cell([time(i), cell2mat(values(log_data, keys))]), 'UniformOutput', false);
        %     fprintf(fid, '%s\n', strjoin(data, ','));
        % end
        % 
        % fclose(fid);
        % status_message = {['Log saved as: ', log_path]};

        should_terminate = true;
    end
end