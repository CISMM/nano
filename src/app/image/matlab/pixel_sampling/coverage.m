function cov = coverage(cl)

% cov = coverage(cl)
% cl is a list of classified pixels

cov = sum(cl(:, 4)) / size(cl, 1);