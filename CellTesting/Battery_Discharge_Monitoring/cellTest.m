% Leonid Pozdneev
% Created January 2019
%
% This function takes the data (put into an .csv file) generated by the
% batteryDischargeMonitoring.ino code. It accepts the name of the .csv file
% as an input, and plots graphs of Voltage vs. Time, Current vs. Time, and
% Resistance vs. Time for all 4 channels with different colors.
%
% To use this function, this file needs to be either in the MATLAB
% directory, or the path to the directory with this file needs to be added
% to MATLAB.

function dataOut = cellTest(filename)
data = csvread(filename);
nColumns = size(data, 1);
nRows = size(data, 2);

dataOut.I1 = []; % array to store values of current from the channel 1
dataOut.V1 = []; % array to store values of voltage from the channel 1
%dataOut.R1 = []; % array to store values of resistance from  the channel 1
dataOut.t1 = []; % array to store values of time for channel 1

dataOut.I2 = []; % array to store values of current from the channel 2
dataOut.V2 = []; % array to store values of voltage from the channel 2
%dataOut.R2 = []; % array to store values of resistance from  the channel 2
dataOut.t2 = []; % array to store values of time for channel 2

dataOut.I3 = []; % array to store values of current from the channel 3
dataOut.V3 = []; % array to store values of voltage from the channel 3
%dataOut.R3 = []; % array to store values of resistance from  the channel 3
dataOut.t3 = []; % array to store values of time for channel 3

dataOut.I4 = []; % array to store values of current from the channel 4
dataOut.V4 = []; % array to store values of voltage from the channel 4
%dataOut.R4 = []; % array to store values of resistance from  the channel 4
dataOut.t4 = []; % array to store values of time for channel 4

for c = 1:nColumns
    for r = 1:nRows

        % identify the channel
        if data(c, 1) == 1
            dataOut.V1 = [dataOut.V1, data(c, 2)]; % record the voltage
            dataOut.I1 = [dataOut.I1, data(c, 3)]; % record the current
            %dataOut.R1 = [dataOut.R1, data(c, 4)]; % record the internal resistance
            dataOut.t1 = [dataOut.t1, data(c, 5)]; % record the rime

        elseif data(c, 1) == 2
            dataOut.V2 = [dataOut.V2, data(c, 2)];
            dataOut.I2 = [dataOut.I2, data(c, 3)];
            %dataOut.R2 = [dataOut.R2, data(c, 4)];
            dataOut.t2 = [dataOut.t2, data(c, 5)];

        elseif data(c, 1) == 3
            dataOut.V3 = [dataOut.V3, data(c, 2)];
            dataOut.I3 = [dataOut.I3, data(c, 3)];
            %dataOut.R3 = [dataOut.R3, data(c, 4)];
            dataOut.t3 = [dataOut.t3, data(c, 5)];

        elseif data(c, 1) == 4
            dataOut.V4 = [dataOut.V4, data(c, 2)];
            dataOut.I4 = [dataOut.I4, data(c, 3)];
            %dataOut.R4 = [dataOut.R4, data(c, 4)];
            dataOut.t4 = [dataOut.t4, data(c, 5)];
        end

    end
end

subplot(3, 1, 1), plot(dataOut.t1, dataOut.V1, 'b-'), hold on
subplot(3, 1, 2), plot(dataOut.t1, dataOut.I1, 'b-'), hold on
subplot(3, 1, 3), plot(dataOut.t1, dataOut.R1, 'b-'), hold on

subplot(3, 1, 1), plot(dataOut.t2, dataOut.V2, 'g-'), hold on
subplot(3, 1, 2), plot(dataOut.t2, dataOut.I2, 'g-'), hold on
subplot(3, 1, 3), plot(dataOut.t2, dataOut.R2, 'g-'), hold on

subplot(3, 1, 1), plot(dataOut.t3, dataOut.V3, 'r-'), hold on
subplot(3, 1, 2), plot(dataOut.t3, dataOut.I3, 'r-'), hold on
subplot(3, 1, 3), plot(dataOut.t3, dataOut.R3, 'r-'), hold on

subplot(3, 1, 1), plot(dataOut.t4, dataOut.V4, 'y-'), hold on
subplot(3, 1, 2), plot(dataOut.t4, dataOut.I4, 'y-'), hold on
subplot(3, 1, 3), plot(dataOut.t4, dataOut.R4, 'y-'), hold off
end