function delta = confdelta(conf, p, n)

% delta = confdelta(conf, p, n)
% Gives the half-width of a confidence interval, expressed
% as a fraction of the estimated value.  For instance, 
%    confdelta(.95, .4, 100) = .2413
% This means that, if you poll 100 people and find that 40% are
% Republicans, then you're 95 percent sure that your estimate is
% off by no more than about 24%.

delta = -norminv((1-conf)/2, 0, 1) * sqrt((1-p) ./ ((n-1)*p));