function int = confint(conf, p, n)

% int = confint(conf, p, n)
% Gives the confidence interval of an estimated proportion of 
% a large population.

d = confdelta(conf, p, n);
D = p * d;
int = [p-D, p+D];