%% Function Usage:
%  Call this function to extract SD CARD data from a vehicle session. The
%  data will be stored as a .mat file in the folder which you run the
%  script.
%
%% Inputs:
%
%  Month:
%  Accepts 1,2,3,4,5,6,7,8,9,10,11,12
%
%  Day:
%  Accepts 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
%
%  Year:
%  Accepts 19,20,21...
%
%% Outputs
%
%  brakeRaw: brake pedal value
%  accelRaw: acceleration pedal value

function [S] = dataExtract(mm, dd, yy, startFileNum, endFileNum)

% Define Data Type Names
dataNames = {};
% Initialize structure
S.BRAKEPEDALRAW = [];

% Get starting time stamp in seconds
%fn_start = string(mm) + '_' + string(dd) + '_' + string(yy) + '_data' + pad(string(startFileNum), 4, 'left', '0') + '.csv';
fn_start = 'DATA' + pad(string(startFileNum), 4, 'left', '0') + '.CSV';
fh_start = fopen(fn_start, 'r');
line = fgetl(fh_start);
line = fgetl(fh_start);
timeStamp = line(12:19);
startTimeSeconds = str2double(timeStamp(1:2))*3600 + str2double(timeStamp(4:5))*60 + str2double(timeStamp(7:end));
fclose(fh_start);

%% Iterate through files
for n = startFileNum : endFileNum
    %% Compose the file name
    %file = string(mm) + '_' + string(dd) + '_' + string(yy) + '_data' + pad(string(n), 4, 'left', '0');
    file = 'DATA' + pad(string(n), 4, 'left', '0');
    fn = file + '.csv'
    
    % Open file and purge first line
    fh = fopen(fn,'r');
    line = fgetl(fh);
    
%     % Get starting time stamp in seconds
%     line = fgetl(fh);
%     timeStamp = line(12:19);
%     startTimeSeconds = str2double(timeStamp(1:2))*3600 + str2double(timeStamp(4:5))*60 + str2double(timeStamp(7:end));
%     fclose(fh);
    
    %% Standard Data Form:
    % 2020-MM-DD HH:MM:SS,<DATA STRING>,<DATA VALUE>,<UNIT>
    % ^    ^  ^  ^  ^  ^  ^            ^            ^
    % 1    6  9  12 15 18 21           VARIES       VARIES
    
    %%
    while true
        line = fgetl(fh);
        if ~ischar(line); break; end
        
        % Get time stamp
        timeStamp = line(12:19);
        % HH:MM:SS to integer seconds
        timeSeconds = str2double(timeStamp(1:2))*3600 + str2double(timeStamp(4:5))*60 + str2double(timeStamp(7:end))-startTimeSeconds;
        [~,num] = strtok(line(30:end),',');
        % Get data value
        [~,num] = strtok(num,',');
        num = strtok(num,',');
        if contains(num,',')
            [~,num] = strtok(num,',');
        end
        num = str2double(num);
        
        % Get the data name
        commas = strfind(line, ',');
        dataName = line(commas(3) + 1 : commas(4) - 1);
        dataName(dataName == ' ') = [];
        if any(strcmp(dataNames, dataName))
            S.(dataName) = [S.(dataName); [timeSeconds, num]];
        else
            S = setfield(S, dataName, [timeSeconds, num]);
            dataNames{length(dataNames) + 1, 1} = dataName;
        end 
    end
end

% interpolate time stamps
for i = 1 : length(dataNames)
    dataName = dataNames{i};
    dataName(dataName == ' ') = [];
    
    startTime = S.(dataName)(1, 1);
    startIndex = 1;
    
    for k = 1 : size(S.(dataName), 1)
        if S.(dataName)(k, 1) ~= startTime
            spacedTimes = linspace(startTime, S.(dataName)(k, 1), k - startIndex + 1);
            S.(dataName)(startIndex : k - 1, 1) = spacedTimes(1 : end - 1);
            startIndex = k;
            startTime = S.(dataName)(k, 1);
        end
    end
end

save_file = string(mm) + '_' + string(dd) + '_' + string(yy) + '_data'
save(save_file + '.mat', 'S');
fclose(fh);
end