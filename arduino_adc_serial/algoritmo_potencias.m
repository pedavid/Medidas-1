clear all
close all
clc
pkg load signal

Vp = 311; Ip = 2; Iar2 = 0.6; Iar3 = 0.3;

%%Time specifications:
Fs = 800;                   % samples per second
dt = 1/Fs;                   % seconds per sample
StopTime = 0.1;             % seconds
t = (0:dt:StopTime-dt)';     % seconds
   
%%Tension:
Fv = 50;                     % hertz
v = Vp*sin(2*pi*Fv*t);
   
%%Corriente:
Fi = 50;
i = Ip*sin(2*pi*Fi*t)+Iar2*sin(2*pi*2*Fi*t)+Iar3*sin(2*pi*3*Fi*t);

##plot(t,i);

vrms=rms(v);
irms=rms(i);

ps=vrms*irms;

pa = 0;
for a=1:1:80
 pa += v(a)*i(a);
endfor

pa /= length(v);

cosphi= pa/ps;