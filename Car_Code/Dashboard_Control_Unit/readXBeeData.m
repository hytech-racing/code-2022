%% Read and Plot Arduino Serial Data
%{
Contact me @ 412-779-3741 or chendrix@gatech.edu with any problems

1)  Establish a link with Teensy via Arduino compiler (you will need to
    have Teensyduino installed). 
    Tools --> Board --> Teensy 3.1/3.2, and
    Tools --> Port  --> ________
2)  Do NOT open the serial monitor on Arduino, or you will not be able to
    read data on MATLAB (conflicting signals)
2)  Take the COM port you are using and change the variable. Mine was
    COM6, for example.
3)  Hit "Run" up top. That should pretty much do it. If some initializer
    error occurs there are two things to do:
    1)  Copy and paste this code, then try running again:
            fclose(arduino);
            delete(arduino);
            clear arduino;
            clear
            clc
        If this does not work then...
    2)  Restart MATLAB! (Sorry)
    Note: This thing has worked with near 100% consistency. Dropouts
    have always been human error.
4)  ******WHEN CLOSING OUT OF THE GUI****** make sure to use the quit
    toggle button up top. If you try using ctrl + c then you're gonna need to
    do step 3.
%}
%% Establish connection with arduino
port = 'COM6';      % CHANGE THIS

try
    arduino = serial(port,'BaudRate',115200);
    fopen(arduino); 
catch
    error('Could not open port. Check if in use by Arduino IDE. May need to restart MATLAB.')
end

%% Create data fields
modA = []; modB = []; modC = []; gate = [];
moto_temp = [];
Motor_Speed = 0; Motor_Angle = [];
A = []; B = []; C = []; DC_I = [];
DC_V = []; Out = []; AB = []; BC = [];
VSM = []; INV_STATE = []; INV_RUN = []; INV_ADS = []; INV_CM = []; INV_EN = 0; LOCK = []; DIR = [];
COM_TQ = []; TQ_FDBK = []; RMS = [];

%% Build GUI background and images
f = figure('Name','CAN Data','Color','white',...
    'units','normalized','outerposition',[0 0 1 1]);

logo = imread('hytech_logo.png');
logo = imrotate(logo,90);
axes('position',[0,0.15,0.12,0.7])
imshow(logo)

%% Save data / Quit Loop Button
% NOTE: do NOT close out of interface WITHOUT using this button. Otherwise,
% you will need to restart MATLAB for the next run. See bottom of code for
% more details.
btn = uicontrol('Style','checkbox',...
                'String','TERMINATE DATA COLLECTION',...
                'Position',[650,735,500,20],...
                'BackgroundColor','white',...
                'FontSize',12);

%% Create speedometer
th = linspace(-0.8,4,1000);
subplot(3,3,2);
plot(cos(th),sin(th),'Color','k','LineWidth',4); hold on
plot(1.2*cos(th),1.2*sin(th),'Color',[0.8000 0.6824 0.3882],'LineWidth',3);
axis equal, axis off
text(-0.2,-0.85,'ENABLE','Color','k','FontSize',12,...
                         'VerticalAlignment','middle',...
                         'HorizontalAlignment','center');
plot(0.5,-0.85,'o','Color',[1-INV_EN(end) INV_EN(end) 0],...
     'MarkerFaceColor',[1-INV_EN(end) INV_EN(end) 0],'MarkerSize',8);
 
 
%% Build GUI
for i = 1:9
subplot(3,3,i)
end
clear i

temp1 = sprintf('TEMPERATURES 1 (%cC)', char(176));
temp3 = sprintf('MOTOR TEMP (%cC)', char(176));

run = true;
L = 50; % Axis parameter (x-axis length before scrolling begins)

%% Read and parse from Arduino serial monitor
while run
     
    CAN_Data = fscanf(arduino, '%c');
    [id, vals] = strtok(CAN_Data);
    vals = str2num(vals);
    if strcmp(id,'ID_MC_TEMPERATURES_1')
        vals = vals/10;
        modA = [modA vals(1)];
        modB = [modB vals(2)];
        modC = [modC vals(3)];
        gate = [gate vals(4)];
        subplot(3,3,7), hold on, 
        plot(modA,'r'), plot(modB,'g'), plot(modC,'b'), plot(gate,'k');
        title(temp1)
        if length(modA) < L
            axis([0 L 0 100]);
        else
            axis([length(modA)-L length(modA) 0 100]);
        end
        legend('modA','modB','modC', 'gate','Location','NorthWest')
    elseif strcmp(id,'ID_MC_TEMPERATURES_3')
        vals = vals/10;
        moto_temp = [moto_temp vals(3)];
        subplot(3,3,8), hold on,
        plot(moto_temp,'k');
        title(temp3)
        if length(moto_temp) < L
            axis([0 L 0 100]);
        else
            axis([length(moto_temp)-L length(moto_temp) 0 100]);
        end
    elseif strcmp(id,'ID_MC_MOTOR_POSITION_INFORMATION')
        Motor_Angle = [Motor_Angle vals(1)/10];
        Motor_Speed = [Motor_Speed vals(2)];
        subplot(3,3,1),hold on,
        %plot(Motor_Angle,'k')
        plot(Motor_Speed','k')
        title('MOTOR SPEED (RPM)')
        if length(Motor_Speed) < L
            axis([0 L 0 6000]);
        else
            axis([length(Motor_Speed)-L length(Motor_Speed) 0 6000]);
        end
        subplot(3,3,2) 
        plot(cos(th),sin(th),'Color','k','LineWidth',4); hold on
        plot(1.2*cos(th),1.2*sin(th),'Color',[0.8000 0.6824 0.3882],'LineWidth',3);
        axis equal, axis off
        r = (225 - Motor_Speed(end))*pi/180;
        phi = linspace(r,4);
        text(0,0,num2str(round(Motor_Speed(end))),'Color','k','FontSize',20,...
            'FontName','DS-Digital',...
            'VerticalAlignment','middle','HorizontalAlignment','center');
        plot(0.5,-0.85,'o','Color',[1-INV_EN(end) INV_EN(end) 0],...
             'MarkerFaceColor',[1-INV_EN(end) INV_EN(end) 0]);
        text(-0.2,-0.85,'ENABLE','Color','k','FontSize',12,...
             'VerticalAlignment','middle',...
             'HorizontalAlignment','center');
        hold off
    elseif strcmp(id,'ID_MC_CURRENT_INFORMATION')
        vals = vals/10;
        A = [A vals(1)];
        B = [B vals(2)];
        C = [C vals(3)];
        DC_I = [DC_I vals(4)];
        subplot(3,3,3), hold on,
        title('CURRENT (A)')
        plot(A,'r'), plot(B, 'g'), plot(C, 'b'), plot(DC_I, 'k')
        if length(A) < L
            axis([0 L -220 220]);
        else
            axis([length(A)-L length(A) -220 220]);
        end
        legend('A','B','C', 'DC BUS','Location','NorthWest')
    elseif strcmp(id,'ID_MC_VOLTAGE_INFORMATION')        
        DC_V = [DC_V vals(1)];
        Out = [Out vals(2)];
        AB = [AB vals(3)];
        BC = [BC vals(4)];
        subplot(3,3,6), hold on,
        title('VOLTAGES (V)')
        plot(DC_V,'r'), plot(Out, 'g'), plot(AB, 'b'), plot(BC, 'k')
        if length(DC_V) < L
            axis([0 L -300 300]);
        else
            axis([length(DC_V)-L length(DC_V) -300 300]);
        end
        legend('DC BUS','OUT','AB', 'BC','Location','NorthWest')
    elseif strcmp(id,'ID_MC_INTERNAL_STATES')
        VSM = [VSM vals(1)];
        INV_STATE = [INV_STATE vals(2)];
        INV_RUN = [INV_RUN vals(3)];
        INV_ADS = [INV_ADS vals(4)];
        INV_CM = [INV_CM vals(5)];
        INV_EN = [INV_EN vals(6)];
        LOCK = [LOCK vals(7)];
        DIR = [DIR vals(8)];
    elseif strcmp(id,'ID_MC_TORQUE_TIMER_INFORMATION')
        vals = vals/10;
        COM_TQ = [COM_TQ vals(1)];
        TQ_FDBK = [TQ_FDBK vals(2)];
        RMS = [RMS vals(3)];
        subplot(3,3,4), hold on,
        title('TORQUE AND RMS UPTIME (N*m)')
        plot(COM_TQ,'r'), plot(TQ_FDBK,'g'), plot(RMS,'b')
        if length(COM_TQ) < L
            axis([0 L -20 90]);
        else
            axis([length(COM_TQ)-L length(COM_TQ) -20 90])
        end
    end
       
    pause(0.01);
    
    if btn.Value == 1
        run = false;
    end
    
end

%% Saving the file
fprintf('Data collection and plotting terminated\n\n');

button = questdlg('Would you like to save this data?');
close(f);
if strcmpi(button,'yes');
    try
        mkdir('CAN_Data');          % Make CAN_Data folder, or add to it
        list = dir('CAN_Data');
        names = {list.name};
        check = strfind(names,date);
        n = length(cell2mat(check));
    catch
        warning('Could not create new folder.')
        fprintf('\n')
    end
        
    S = date;
    [Path,Folder]=fileparts(pwd);
    if ispc
        title = sprintf('%s\\%s\\CAN_Data\\%s(%d).txt',Path,Folder,S,n);
    elseif ismac
        title = sprintf('%s//%s//CAN_Data//%s(%d).txt',Path,Folder,S,n);
    end
    try
        fh = fopen(title,'w');
    catch
        save_loc_error = true;
        title = sprintf('%s.txt',S);
        fh = fopen(title,'w');
    end
    fprintf(fh,'%s\n',datestr(now));
    fprintf(fh,'Module A Temp: %s\n', num2str(modA));
    fprintf(fh,'Module B Temp: %s\n', num2str(modB));
    fprintf(fh,'Module C Temp: %s\n', num2str(modC));
    fprintf(fh,'Gate Temp: %s\n', num2str(gate));
    fprintf(fh,'Motor Temp: %s\n', num2str(moto_temp));
    fprintf(fh,'Motor Speed: %s\n', num2str(Motor_Speed));
    fprintf(fh,'Motor Angle: %s\n', num2str(Motor_Angle));
    fprintf(fh,'Current A: %s\n', num2str(A));
    fprintf(fh,'Current B: %s\n', num2str(B));
    fprintf(fh,'Current C: %s\n', num2str(C));
    fprintf(fh,'DC BUS Current: %s\n', num2str(DC_I));
    fprintf(fh,'DC BUS Voltage: %s\n', num2str(DC_V));
    fprintf(fh,'OUT Voltage: %s\n', num2str(Out));
    fprintf(fh,'AB Voltage: %s\n', num2str(AB));
    fprintf(fh,'BC Voltage: %s\n', num2str(BC));
    fprintf(fh,'VSM State: %s\n', num2str(VSM));
    fprintf(fh,'Inverter State: %s\n', num2str(INV_STATE));
    fprintf(fh,'Inverter Run Mode: %s\n', num2str(INV_RUN));
    fprintf(fh,'Inverter Active Discharge State: %s\n', num2str(INV_ADS));
    fprintf(fh,'Inverter Command State: %s\n', num2str(INV_CM));
    fprintf(fh,'Inverter Enable: %s\n', num2str(INV_EN));
    fprintf(fh,'Inverter Lockout: %s\n', num2str(LOCK));
    fprintf(fh,'Direction Command: %s\n', num2str(DIR));
    fprintf(fh,'Commanded Torque: %s\n', num2str(COM_TQ));
    fprintf(fh,'Torque Feedback: %s\n', num2str(TQ_FDBK));
    fprintf(fh,'RMS Uptime: %s\n', num2str(RMS));
    fclose(fh);
end

%% Preparing for next test
% If this is not run before initializing next test, you will need to
% restart MATLAB
fclose(arduino);
delete(arduino);
clear arduino;
clear
clc
if save_loc_error
    warning(['Could not write new file to CAN_data subfolder. ',...
             'Saved to current directory instead.'])
end
