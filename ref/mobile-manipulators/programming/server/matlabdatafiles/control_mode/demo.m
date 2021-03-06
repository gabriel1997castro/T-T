clear all;
close all;

FlagGenerateNewDataFile = 1;

if FlagGenerateNewDataFile
    Filename = 'gmatlabdatafile.mat';
    t = ReadGMatlabDataFile('t',Filename);		%% time
    T_exec = ReadGMatlabDataFile('T_exec',Filename);		%% execution time
    gyro_1 = ReadGMatlabDataFile('g_1',Filename);	%% gyro 1
    gyro_2 = ReadGMatlabDataFile('g_2',Filename);	%% gyro 2
    i_1 = ReadGMatlabDataFile('i_k',Filename);	%% motor current knee
    i_2 = ReadGMatlabDataFile('i_as',Filename);	%% motor current ankle sagital
    i_3 = ReadGMatlabDataFile('i_af',Filename);	%% motor current ankle frontal
    u_1 = ReadGMatlabDataFile('u_k',Filename);	%% motor pwm input [0-1000]	- knee
    u_2 = ReadGMatlabDataFile('u_as',Filename);	%% motor pwm input [0-1000]	- ankle sagital
    u_3 = ReadGMatlabDataFile('u_af',Filename);	%% motor pwm input [0-1000]	- ankle frontal
    theta_1 = ReadGMatlabDataFile('p_k',Filename);	%% knee joint potentiometer
    theta_2 = ReadGMatlabDataFile('p_as',Filename);	%% knee joint potentiometer
    theta_3 = ReadGMatlabDataFile('p_af',Filename);	%% knee joint potentiometer
    r_1 = ReadGMatlabDataFile('r_1',Filename);	%% foot infrared range 1
    r_2 = ReadGMatlabDataFile('r_2',Filename);	%% foot infrared range 2
    r_3 = ReadGMatlabDataFile('r_3',Filename);	%% foot infrared range 3
    r_4 = ReadGMatlabDataFile('r_4',Filename);	%% foot infrared range 4
    
    %%% converts motor inputs to Volts
    u_1 = 12*(u_1 - 500)/500;
    u_2 = -12*(u_2 - 500)/500;
    u_3 = 12*(u_3 - 500)/500;
    
    %%% converts currents inputs to Ampers
    i_1 = (i_1*3.3/1023 - 2.5)/0.133;
    i_2 = -(i_2*3.3/1023 - 2.5)/0.133;
    i_3 = (i_3*3.3/1023 - 2.5)/0.133;
    
    %%% converts knee potentiometer readings to rad
    k = 1;
    a = 0;
    theta_1 = k*theta_1 + a;
    
    %%% converts ankle potentiometer readings to rad
    output_cal = [0; pi/2   ];
    input_cal  = [604;  920  ];
    k = (output_cal(2)-output_cal(1))/(input_cal(2)-input_cal(1));
    a = output_cal(1)-k*input_cal(1);
    theta_2 = k*theta_2 + a;
    
    %%% converts foot potentiometer readings to rad
    output_cal = [0; pi/2   ];
    input_cal  = [476;  960  ];
    k = (output_cal(2)-output_cal(1))/(input_cal(2)-input_cal(1));
    a = output_cal(1)-k*input_cal(1);
    theta_3 = k*theta_3 + a;
    
    %%% converts foot pitch giro readings to rad/s
    i_cal = min(find(t>=1)); %compute bias
    gyro_1 = gyro_1 - mean(gyro_1(1:i_cal)); % compensate bias
    gyro_1 = gyro_1 * 3.3/1024; % convert to Volt
    gyro_1 = (gyro_1 / 0.0125)*pi/180.0; % convert to rad/s according ro ADXRS300 datasheet
    
    %%% converts foot roll giro readings to rad/s
    i_cal = min(find(t>=1)); %compute bias
    gyro_2 = gyro_2 - mean(gyro_2(1:i_cal)); % compensate bias
    gyro_2 = gyro_2 * 3.3/1024; % convert to Volt
    gyro_2 = (gyro_2 / 0.005)*pi/180.0; % convert to rad/s according ro ADXRS150 datasheet
    
    %%% converts infrared rangefinder readings to m
    r_1 = r_1*3.3/255; % convert to V
    r_2 = r_2*3.3/255; % convert to V
    r_3 = r_3*3.3/255; % convert to V
    r_4 = r_4*3.3/255; % convert to V
   
    r_1 = 0.01*(13.2./r_1 - 0.42); % convert to m
    r_2 = 0.01*(13.2./r_2 - 0.42); % convert to m
    r_3 = 0.01*(13.2./r_3 - 0.42); % convert to m
    r_4 = 0.01*(13.2./r_4 - 0.42); % convert to m

    r_1(find(r_1 > 0.4)) = 0.4;
    r_2(find(r_2 > 0.4)) = 0.4;
    r_3(find(r_3 > 0.4)) = 0.4;
    r_4(find(r_4 > 0.4)) = 0.4;
     
    %%% save to file
    save data.mat t gyro_1 gyro_2 u_1 u_2 u_3 i_1 i_2 i_3 theta_1 theta_2 theta_3 r_1 r_2 r_3 r_4;
else
    load data.mat;
end

figure; 
subplot(211), plot(t,gyro_1*180/pi); title('gyro 1'); ylabel('[deg/s]');
subplot(212), plot(t,gyro_2*180/pi); title('gyro 2'); ylabel('[deg/s]');

figure; 
subplot(311), plot(t,u_1); title('voltage motor 1'); ylabel('[V]');
subplot(312), plot(t,u_2); title('voltage motor 2'); ylabel('[V]');
subplot(313), plot(t,u_3); title('voltage motor 3'); ylabel('[V]');

figure; 
subplot(311), plot(t,i_1); title('current motor 1'); ylabel('[A]');
subplot(312), plot(t,i_2); title('current motor 2'); ylabel('[A]');
subplot(313), plot(t,i_3); title('current motor 3'); ylabel('[A]');

figure; 
subplot(311), plot(t,theta_1*180/pi); title('angle joint 1'); ylabel('[deg]');
subplot(312), plot(t,theta_2*180/pi); title('angle joint 2'); ylabel('[deg]');
subplot(313), plot(t,theta_3*180/pi); title('angle joint 3'); ylabel('[deg]');

figure; 
subplot(411), plot(t,r_1); title('infrared rangefinder 1'); ylabel('[m]');
subplot(412), plot(t,r_2); title('infrared rangefinder 2'); ylabel('[m]');
subplot(413), plot(t,r_3); title('infrared rangefinder 3'); ylabel('[m]');
subplot(414), plot(t,r_4); title('infrared rangefinder 4'); ylabel('[m]');

figure; 
subplot(311), plot(t,[0; (diff(theta_1)./diff(t))*180/pi]); title('angular speed joint 1 (estimate 1st order)'); ylabel('[deg/s]');
subplot(312), plot(t,[0; (diff(theta_2)./diff(t))*180/pi]); title('angular speed joint 2 (estimate 1st order)'); ylabel('[deg/s]');
subplot(313), plot(t,[0; (diff(theta_3)./diff(t))*180/pi]); title('angular speed joint 3 (estimate 1st order)'); ylabel('[deg/s]');

figure; 
subplot(211), plot(t,[0; diff(t)]); title('Sampling time'); ylabel('[s]');
subplot(212), plot(t,[0; diff(t)],'r',t,T_exec,'b'); title('Execution time (blue) vs. Sampling time (red)'); ylabel('[s]');

