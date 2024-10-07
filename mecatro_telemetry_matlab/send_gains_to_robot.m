function send_gains_to_robot(gains, socket)
    % send_gains_to_robot: Send gains to robot over a socket connection.
    %   - gains: array of float values to send
    %   - socket: TCP/IP socket object

    % Prepare byte array to send
    send_array = zeros(1, 4 * length(gains) + 2, 'uint8');
    send_array(1) = 255;  % Start byte

    % Pack the float32 gains into the byte array
    for i = 1:length(gains)
        gain_bytes = typecast(single(gains(i)), 'uint8');  % Convert float32 to 4 bytes
        send_array(4 * (i - 1) + 2 : 4 * i + 1) = gain_bytes;  % Assign the bytes
    end

    % Calculate the checksum
    checksum = mod(sum(send_array(2:end-1)), 256);  % Sum all bytes except start and end
    send_array(end) = 255 - checksum;  % Append checksum

    % Print the send_array for debugging
    disp('Sending:');
    disp(send_array);

    % Send the data over the socket
    write(socket, send_array, 'uint8');
end
