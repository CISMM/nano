function delete_marker(A, dot_is_on)

num_handles = prod(size(A))-1;

for n = 1:num_handles
   delete(A(n));
end

if dot_is_on == 1
   delete(A(end));
end

