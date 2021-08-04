clear all
close all
clc
pkg load signal

signal = ((load("putty.log"))*0.1875/1000-2.3448)*5.4;

L = length(signal);

## Frecuencia de muestreo de 500 Hz

Fs = 500;
dt = 1/Fs;
StopTime = dt * L;
t = (0:dt:StopTime-dt)';

v = 311*cos(2*pi*50*t); 

##Y = fft(signal);
##
##P2 = abs(Y/L);
##P1 = P2(1:L/2+1);
##P1(2:end-1) = 2*P1(2:end-1);
##
##f = Fs*(0:(L/2))/L;
##plot(f,P1); 

##plot(t,signal);

i_eficaz = rms(signal);
v_eficaz = rms(v);

Ps = i_eficaz*v_eficaz;

Pa = 0;
for i=1:L
  Pa += v(i)*signal(i);
endfor

Pa /= L;

Pi = v.*signal;

##plot(t,Pi);

##%%Time specifications:
##   Fs = 8000;                   % samples per second
##   dt = 1/Fs;                   % seconds per sample
##   StopTime = 0.25;             % seconds
##   t = (0:dt:StopTime-dt)';     % seconds
##   %%Sine wave:
##   Fc = 50;                     % hertz
##   x = 311*sin(2*pi*Fc*t);

