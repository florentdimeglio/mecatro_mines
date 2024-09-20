function sendGainsToRobot(gains, serial)
    % Send gains to robot.
    %   - gains: vector of single precision floats to send
    %   - serial: serial object
    
    sendArray = uint8(zeros(1, 4 * length(gains) + 2));
    sendArray(1) = hex2dec('FF');
    
    for i = 1:length(gains)
        floatBytes = typecast(single(gains(i)), 'uint8');
        startIndex = 4 * (i - 1) + 2;
        endIndex = 4 * i + 1;
        sendArray(startIndex:endIndex) = floatBytes;
    end
    
    checksum = mod(sum(sendArray(2:end-1)), 256);
    sendArray(end) = checksum;
    
    fwrite(serial, sendArray);
end
