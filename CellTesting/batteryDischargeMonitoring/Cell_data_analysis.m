%% Import Variables
clear;
%clc;

files = dir('*.txt');
load('Cell_Testing_Data.mat')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Update Cell List With New Tests %%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Cell_List = ... %Cell ID, File/Test Number, Channel Number
    {...
    'c19076', 1, 1;...%1
    'c19077', 1, 2;...%2
    ...
    'c19078', 2, 1;...%3
    'c19079', 2, 2;...%4
    ...
    'c19082', 3, 1;...%5
    'c19083', 3, 2;...%6
    ...
    'c19084', 4, 1;...%7
    'c19085', 4, 2;...%8
    ...
    'c19086', 5, 1;...%9
    'c19087', 5, 2;...%10
    ...
    'c19088', 6, 1;...%11
    'c19089', 6, 2;...%12
    ...
    'c19038', 7, 1;...%13
    'c19039', 7, 2;...%14
    'c19094', 7, 3;...%15
    ...
    'c19037', 8, 1;...%16
    'c19040', 8, 2;...%17
    'c19041', 8, 3;...%18
    'c19043', 8, 4;...%19
    };
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if ~exist('CellData','var')
    CellData = struct;
end

[rows, ~] = size(Cell_List);
f = waitbar(0, 'Importing Cell Testing Data...');
for c = 1:2 % IR expressed in ohms
    ID = Cell_List{c,1}; % Cell ID
    i =  Cell_List{c,2}; %file number
    j =  Cell_List{c,3}; %channel number
    waitbar(c/rows, f)
    if ~isfield(CellData,ID)
        tab_import(i).raw = tdfread(files(i).name,'\t');
        CellData.(ID) =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
                          tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR*1000, ...
                          tab_import(i).raw.Contactor, tab_import(i).raw.State];
        CellData.(ID) =  CellData.(ID)(tab_import(i).raw.Channel == j,:);
    end
end
for c = 3:12 % IR expressed in mOhms
    waitbar(c/rows, f)
    ID = Cell_List{c,1}; % Cell ID
    i =  Cell_List{c,2}; %file number
    j =  Cell_List{c,3}; %channel number
    if ~isfield(CellData,ID)
        tab_import(i).raw = tdfread(files(i).name,'\t');
        CellData.(ID) =  [tab_import(i).raw.Time/1000,...%1
                          tab_import(i).raw.Voltage,...%2
                          tab_import(i).raw.Current, ...%3
                          tab_import(i).raw.OCV_Predict,...%4
                          tab_import(i).raw.Calculated_IR_mOhm,...%5
                          tab_import(i).raw.Contactor,...%6
                          tab_import(i).raw.State];%7
        CellData.(ID) =  CellData.(ID)(tab_import(i).raw.Channel == j,:);
    end    
end
for c = 13:rows % V_avg and Rest Periods
    waitbar(c/rows, f)
    ID = Cell_List{c,1}; % Cell ID
    i =  Cell_List{c,2}; %file number
    j =  Cell_List{c,3}; %channel number
    if ~isfield(CellData,ID)
        tab_import(i).raw = tdfread(files(i).name,'\t');
        CellData.(ID) =  [tab_import(i).raw.Time/1000,...%1
                          tab_import(i).raw.Voltage,...%2
                          tab_import(i).raw.Current, ...%3
                          tab_import(i).raw.OCV_Predict,...%4
                          tab_import(i).raw.Calculated_IR_mOhm,...%5
                          tab_import(i).raw.Contactor,...%6
                          tab_import(i).raw.State,...%7
                          tab_import(i).raw.V_avg,...%8
                          tab_import(i).raw.I_avg,...%9
                          tab_import(i).raw.Rest_Periods];%10
        CellData.(ID) =  CellData.(ID)(tab_import(i).raw.Channel == j,:);
    end    
end

close(f)

clear files i j tab_import ID ans f c
save('Cell_Testing_Data.mat')

%% Coulomb Counting
figure(1);clf
CellSummary.V_Ah_table = zeros(rows,4);
[rows, ~] = size(Cell_List);

Cumulative_Ah_col = 11;

for c = 1:rows
    ID = Cell_List{c,1}; % Cell ID
    
    CellData.(ID)( (CellData.(ID)(:,6) == 0) ,3) = 0; % force zero current when contactor is open
    CellData.(ID)(:,11) = (1/ 3600) .* cumtrapz( CellData.(ID)(:,1) ,  CellData.(ID)(:,3) ); % Amp-Seconds to Amp-Hour Conversion
    
    CellSummary.(ID).StartV = mean(CellData.(ID)(1:100,2)); %average of first 100 measurements
    CellSummary.(ID).Ah = CellData.(ID)(end,Cumulative_Ah_col); %Last Cumulative Sum
    CellSummary.(ID).IR = CellData.(ID)(end,5); %Last Calculated IR
    
    fprintf(['Cell ',ID(2:end), ' ',...
             'Starting Voltage: %0.4f\t',...
             'Ah: %0.4f\t' ...
             'IR (mOhm): %0.4f\n'],CellSummary.(ID).StartV,CellSummary.(ID).Ah,CellSummary.(ID).IR)
    
    scatter(CellSummary.(ID).StartV,CellSummary.(ID).Ah,'Filled','DisplayName',ID); hold on; grid on;legend('Location','NorthWest');
    text(CellSummary.(ID).StartV,CellSummary.(ID).Ah,ID);hold on
    CellSummary.V_Ah_table(c,:) = [str2double(ID(2:end)),CellSummary.(ID).StartV,CellSummary.(ID).Ah,CellSummary.(ID).IR];
end
xlabel('Starting Voltage')
ylabel('Capacity (Ah)')

clear c ID
save('Cell_Testing_Data.mat')

%% OCV vs. SOC
% 4.5C Discharge

figure(3);clf
for c = 13:15
    ID = Cell_List{c,1}; % Cell ID
    
    CellDataExt.(ID).trigger = diff(CellData.(ID)(:,6)); %signchange right before contactor change
    CellDataExt.(ID).upslope = find(CellDataExt.(ID).trigger == 1);
    CellDataExt.(ID).downslope = find(CellDataExt.(ID).trigger == -1);
    
    % Delete Data splice Location
    CellDataExt.(ID).upslope([51,52],:) = [];
    CellDataExt.(ID).downslope([51,52],:) = [];
    
    sample_win = 100;
    CellDataExt.(ID).OCV = [];
    for i = 1:sample_win %last sample window averaging
    CellDataExt.(ID).OCV(:,i) = CellData.(ID)(CellDataExt.(ID).upslope-(i-1),2);
    end
    CellDataExt.(ID).OCV = mean(CellDataExt.(ID).OCV,2);
    CellDataExt.(ID).OCV(end+1) = mean(CellData.(ID)(end-sample_win,8)); %add ending voltage
    
    CellDataExt.(ID).Ah = CellData.(ID)(CellDataExt.(ID).upslope,Cumulative_Ah_col);
    CellDataExt.(ID).Ah(end+1) = CellData.(ID)(end,Cumulative_Ah_col); % add final final Ah count
    CellDataExt.(ID).SOD = CellDataExt.(ID).Ah / CellData.(ID)(end,Cumulative_Ah_col);
    CellDataExt.(ID).SOC = (1-CellDataExt.(ID).SOD)*100; %convert SOD to Percent SOC
    
    sample_win = 10;
    CellDataExt.(ID).CCV0 = [];
    for i = 1:sample_win
    CellDataExt.(ID).CCV0(:,i) = CellData.(ID)(CellDataExt.(ID).upslope+3+(i-1),2); %voltage immediately after contactor closes
    end
    CellDataExt.(ID).CCV0 = mean(CellDataExt.(ID).CCV0,2);
    
    sample_win = 10;
    CellDataExt.(ID).I0 = [];
    for i = 1:sample_win
    CellDataExt.(ID).I0(:,i) = CellData.(ID)(CellDataExt.(ID).upslope+3+(i-1),3); %current immediately after contactor closes
    end
    CellDataExt.(ID).I0 = mean(CellDataExt.(ID).I0,2);
    
    CellDataExt.(ID).IR0 = (CellDataExt.(ID).OCV(1:end-1) - CellDataExt.(ID).CCV0) ./ CellDataExt.(ID).I0; %instantaneous IR
    
    sample_win = 100;
    CellDataExt.(ID).CCV2 = [];
    for i = 1:sample_win
    CellDataExt.(ID).CCV2(:,i) = CellData.(ID)(CellDataExt.(ID).downslope-(i-1),2); %voltage immediately before contactor opens
    end
    CellDataExt.(ID).CCV2 = mean(CellDataExt.(ID).CCV2,2);
    
    sample_win = 100;
    CellDataExt.(ID).I2 = [];
    for i = 1:sample_win
    CellDataExt.(ID).I2(:,i) = CellData.(ID)(CellDataExt.(ID).downslope-(i-1),3); %current immediately before contactor opens
    end
    CellDataExt.(ID).I2 = mean(CellDataExt.(ID).I2,2);
    
    CellDataExt.(ID).IR2 = (CellDataExt.(ID).OCV(2:end) - CellDataExt.(ID).CCV2) ./ CellDataExt.(ID).I2; %R0 + R1 total internal resistance
    
    subplot(2,1,1)
    scatter((CellDataExt.(ID).SOC),CellDataExt.(ID).OCV,'.','DisplayName',ID);hold on;grid on;legend('Location','SouthEast');
    title('Battery Open Circuit Voltage vs. State of Charge %')
    xlabel('SOC (%)')
    ylabel('Battery OCV')
    subplot(2,1,2)
    plot((CellDataExt.(ID).Ah),CellDataExt.(ID).OCV,'DisplayName',ID);hold on;grid on;legend;
    title('Battery Open Circuit Voltage vs. Discharge Energy')
    xlabel('Cumulative Amp-Hours (Ah)')
    ylabel('Battery OCV')
end

CellDataExt.MasterOCV = [CellDataExt.c19038.OCV;CellDataExt.c19039.OCV;CellDataExt.c19094.OCV] ;
CellDataExt.MasterSOC = [CellDataExt.c19038.SOC;CellDataExt.c19039.SOC;CellDataExt.c19094.SOC] ;
[CellDataExt.OCVFit,CellDataExt.OCVFitS,CellDataExt.OCVFitmu] = polyfit(CellDataExt.MasterSOC,CellDataExt.MasterOCV,8);

CellDataExt.OCVtable = [linspace(-3,103,1000)',polyval(CellDataExt.OCVFit,linspace(-3,103,1000),[],CellDataExt.OCVFitmu)'];
subplot(2,1,1)
plot(CellDataExt.OCVtable(:,1),CellDataExt.OCVtable(:,2),'b','LineWidth',2','DisplayName','Polynomial Fit')
xlim([-3,103])

figure(4);clf
subplot(2,1,1)
plot(CellDataExt.c19038.OCV(1:end-1),CellDataExt.c19038.IR0.*1e3,'LineWidth',1.5,'DisplayName','c19038'); hold on; grid on; legend;
plot(CellDataExt.c19039.OCV(1:end-1),CellDataExt.c19039.IR0.*1e3,'LineWidth',1.5,'DisplayName','c19039')
plot(CellDataExt.c19094.OCV(1:end-1),CellDataExt.c19094.IR0.*1e3,'LineWidth',1.5,'DisplayName','c19094')
title('Internal Resistance (R0) vs. Open Circuit Voltage')
xlabel('OCV (Volts)')
ylabel('Internal Resistance R0 (m\Omega)')
subplot(2,1,2)
plot(CellDataExt.c19038.OCV(2:end),CellDataExt.c19038.IR2.*1e3,'LineWidth',1.5,'DisplayName','c19038'); hold on; grid on; legend;
plot(CellDataExt.c19039.OCV(2:end),CellDataExt.c19039.IR2.*1e3,'LineWidth',1.5,'DisplayName','c19039')
plot(CellDataExt.c19094.OCV(2:end),CellDataExt.c19094.IR2.*1e3,'LineWidth',1.5,'DisplayName','c19094')
title('Total Internal Resistance (R0+R1) vs. Open Circuit Voltage')
xlabel('OCV (Volts)')
ylabel('Total Internal Resistance (R0+R1) (m\Omega)')

clear i filter_order filter_frame ID rows sample_win ans c %Cumulative_Ah_col
% save('Cell_Testing_Data.mat')

%% Plotting
%{
figure(2);clf
subplot(1,1,1)
plot(CellData.c19076(:,1),CellData.c19076(:,2)); hold on
plot(CellData.c19077(:,1),CellData.c19077(:,2));
plot(CellData.c19078(:,1),CellData.c19078(:,2));
plot(CellData.c19079(:,1),CellData.c19079(:,2));
plot(CellData.c19082(:,1),CellData.c19082(:,2));
plot(CellData.c19083(:,1),CellData.c19083(:,2));
% subplot(2,1,2)
% plot(CellData.c19079(:,1),CellData.c19079(:,3))
% subplot(3,1,3)
% plot(c19079(:,1),c19079(:,6))

figure(3);clf
subplot(1,1,1)
plot(CellData.c19038(:,1),CellData.c19038(:,2)); hold on
plot(CellData.c19039(:,1),CellData.c19039(:,2));
plot(CellData.c19094(:,1),CellData.c19094(:,2));
grid on; legend('c19038','c19039','c19094')
xlabel('Time')
ylabel('Volts')

figure(4);clf
subplot(1,1,1)
plot(CellData.c19037(:,1),CellData.c19037(:,2)); hold on
plot(CellData.c19040(:,1),CellData.c19040(:,2));
plot(CellData.c19041(:,1),CellData.c19041(:,2));
plot(CellData.c19043(:,1),CellData.c19043(:,2));
grid on; legend('c19038','c19039','c19094')
xlabel('Time')
ylabel('Volts')



%% Simulink

V_in = timeseries(CellData.c19076(:,2),CellData.c19076(:,1));
I_in = timeseries(CellData.c19076(:,3),CellData.c19076(:,1));
%}