function n = samp(conf, delta, p)

% n = samp(conf, delta, p)
% Tells how many people you should poll to be off by no more 
% than delta, assuming that the value you get is p.  100delta is a
% percentage of p.

n = norminv((1-conf)/2, 0, 1)^2 / delta^2 * (1-p) ./ p;